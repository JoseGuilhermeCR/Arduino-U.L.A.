use std::collections::HashMap;
use std::{io, fs, process, env};
use std::process::Command;

fn main() {
    let mnemonicos_map: HashMap<&str, char> = [
        ("zeroL", '0'),		// 0 lógico
		("umL", '1'),		// 1 lógico
		("An", '2'),		// A'
		("Bn", '3'),		// B'
		("AouB", '4'),		// A + B
		("AeB", '5'),		// A . B
		("AxorB", '6'),		// A ⊕ B
		("AnandB", '7'),	// (A . B)'
		("AnorB", '8'),		// (A + B)'
		("AxnorB", '9'),	// (A ⊕ B)'
		("AnouB", 'A'), 	// A' + B
		("AouBn", 'B'),		// A + B'
		("AneB", 'C'),		// A' . B
		("AeBn", 'D'),		// A . B'
		("AnouBn", 'E'),	// A' + B'
		("AneBn", 'F')		// A' . B'
    ].iter().collect();

    let mut filename = String::from("testeula.ula");
    let mut porta_com = String::new();

    let args: Vec<String> = env::args().collect();

    if args.len() >= 2 {
        if args[1] == "--help" {
            mostrar_uso();
            process::exit(0);
        } else if args[1] == "--mnemonics" {
            mostrar_mnemonics(&mnemonicos_map);
            process::exit(0);
        } else {
            filename = args[1].clone();
            if args.len() == 3 {
                porta_com = args[2].clone();
            }
        }
    } else {
        println!("Nenhum arquivo passado como argumento. Iniciando modo interativo.");
        println!("Esse código será salvo em {}.", filename);
        escreve_codigo(&filename);
    }

    // Lê o arquivo com o código
    let codigo = fs::read_to_string(&filename)
                    .expect("Erro ao ler arquivo.");

    println!("===Arquivo lido===");
    print!("{}", codigo);
    println!("==================");

    // Gera expressões a partir de um pré-processamento do código, separadas pelo ; e sem espaçoes em branco.
    let expressoes = parse_codigo(codigo);

    // Gera as instruções apartir das expressões.
    let instrucoes = gerar_instrucoes(expressoes, &mnemonicos_map);

    println!("===Instruções geradas===");
    for instrucao in &instrucoes {
        println!("{}", instrucao);
    }
    println!("========================");

    // Escreve as instruções geradas em arquivo .hex
    let dot_offset = filename.find(".").unwrap_or(filename.len());
    filename.replace_range(dot_offset.., ".hex");
    escrever_vetor_em_arquivo(&instrucoes, &filename);
    println!("Instruções gravadas em {}.", filename);

    // Execução
    println!("Quer realizar a execução passo a passo? (s/n): ");
    let mut resposta = String::new();
    io::stdin().read_line(&mut resposta).expect("Erro ao ler resposta.");

    if resposta.to_lowercase().contains("s") {
        if porta_com.is_empty() {
            println!("Digite a porta COM em que o arduino está conectado: ");
            io::stdin().read_line(&mut porta_com).expect("Erro ao ler porta com.");
        }

        println!("Iniciando execução passo a passo:");
        let mut lixo = String::new();
        for instrucao in instrucoes {
            println!("Instrução sendo executada agora: {}", instrucao);

            // Enviamos instrução para o arduino
            Command::new("envia.exe")
                    .args(&[&porta_com, &instrucao])
                    .output()
                    .expect("Algum erro ocorreu!");

            println!("Pressione enter para ir para a próxima instrução.");
            io::stdin().read_line(&mut lixo);
        }
    }
}

fn gerar_instrucoes(expressoes: Vec<String>, mnemonicos_map: &HashMap<&'static str, char>) -> Vec<String> {
    // Instrucões geradas pelas expressões ficarão armazenadas nesse vetor.
    let mut instrucoes = vec![];

    // Só há dois "registradores" nessa máquina, e as instruções só se aplicam a eles.
    let (mut a, mut b) = ('0', '0');

    // Para cada expressão.
    for expressao in expressoes {
        // Por enquanto, ignore inicio e fim.
        if expressao == "fim" || expressao == "inicio" {
            continue;
        }
        
        match expressao.find("=") {
            Some(pos) => {
                let exp = expressao.as_bytes();
                if exp[0] as char == 'A' {
                    a = exp[pos + 1] as char;
                } else {
                    b = exp[pos + 1] as char;
                }
            },
            None => {
                let mut instrucao = String::new();

                instrucao.push(a);
                instrucao.push(b);

                let mnemonico_hex = mnemonicos_map.get(&expressao[..])
                                    .expect("Erro: Foi encontrado um mnemonico que não existe!");
                
                instrucao.push(*mnemonico_hex);

                instrucoes.push(instrucao);
            },
        }
    }

    instrucoes
}

/* Funciona como um filtro para as expressões de um código pego em um arquivo.
   Tira os espaços em branco e as separa de acordo com os tokens . ; e : */
fn parse_codigo(codigo: String) -> Vec<String> {
    let mut expressoes_parsed = vec![];

    // Coleta as palavras separadas em espaço em branco.
    let codigo: String = codigo.split_whitespace().collect();
    
    // Coloca em um vetor separado por ; : e .
    let mut tmp = String::new();
    for caractere in codigo.chars() {
        match caractere {
            ';' | ':' | '.' => {
                expressoes_parsed.push(tmp.clone());
                tmp.clear();
            },
            _ => tmp.push(caractere),
        }
    }

    expressoes_parsed
}

/* No modo interativo, comandos são escritos em um arquivo que será
   posteriormente executado. */
fn escreve_codigo(filename: &String) {
    println!("inicio: ");

    let mut codigo = String::from("inicio:\n");
    while !codigo.contains("fim.") {
        io::stdin().read_line(&mut codigo)
            .expect("Algum erro ocorreu durante a leitura.");
    }

    fs::write(filename, codigo)
        .expect("Erro ao escrever no arquivo.");
}

fn escrever_vetor_em_arquivo(vetor: &Vec<String>, filename: &String) {
    let mut conteudo = String::new();

    for string in vetor {
        conteudo.push_str(string);
        conteudo.push('\n');
    }

    fs::write(filename, conteudo)
        .expect("Erro ao escrever no arquivo.");

}

fn mostrar_uso() {
    println!("Modos de uso: ");
    println!("compiler");
    println!("compiler <arquivo.ula>");
    println!("compiler <arquivo.ula> <porta_com>");
}

fn mostrar_mnemonics(mnemonics_map: &HashMap<&'static str, char>) {
    println!("Todos os mnemônicos disponíveis: ");
    for key in mnemonics_map.keys() {
        println!("{}", key);
    }
}
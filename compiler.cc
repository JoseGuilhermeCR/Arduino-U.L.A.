#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <fstream>
#include <exception>
#include <cstdlib> // std::system
#include <cctype>  // std::tolower

using std::string;
using std::unordered_map;
using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::ifstream;
using std::ofstream;

vector<string> parse_codigo(const string &codigo);
vector<string> gerar_instrucoes(const vector<string> &expressoes);
string uniform(const string &comando);
void write_commands(const string &fileName);
string ler_codigo_de_arquivo(const string &fileName);
void escrever_vetor_em_arquivo(const vector<string> &vetor, const string &filename);

const unordered_map<string, char> mnemonicos_to_hex = {
		{"zeroL", '0'},		// 0 lógico
		{"umL", '1'},		// 1 lógico
		{"An", '2'},		// A'
		{"Bn", '3'},		// B'
		{"AouB", '4'},		// A + B
		{"AeB", '5'},		// A . B
		{"AxorB", '6'},		// A ⊕ B
		{"AnandB", '7'},	// (A . B)'
		{"AnorB", '8'},		// (A + B)'
		{"AxnorB", '9'},	// (A ⊕ B)'
		{"AnouB", 'A'}, 	// A' + B
		{"AouBn", 'B'},		// A + B'
		{"AneB", 'C'},		// A' . B
		{"AeBn", 'D'},		// A . B'
		{"AnouBn", 'E'},	// A' + B'
		{"AneBn", 'F'}		// A' . B'
};

int main(int argc, char **argv) {
	string filename = "testeula.ula";
	string porta_COM;

	// Checamos se um arquivo específico foi passado, se não, iniciamos modo interativo onde o código deve ser escrito.
	if (argc >= 2) {
		string argument(argv[1]);
		if (argument == "--help") {
			cout << "Modos de uso: " << endl;
			cout << argv[0] << endl;
			cout << argv[0] << " <arquivo.ula>" << endl;
			cout << argv[0] << " <arquivo.ula> <porta_com>" << endl;

			return 0;
		} else if (argument == "--mnemonics") {
			cout << "Todos os mnemônicos disponíveis: " << endl;

			for (const auto &mnemonico : mnemonicos_to_hex) {
				cout << mnemonico.first << endl;
			}

			return 0;
		} else {
			filename = argument;
			if (argv[2] != nullptr)
				porta_COM = string(argv[2]);
		}
	} else {
		cout << "Nenhum arquivo passado como argumento. Iniciando modo interativo." << endl;
		cout << "Esse código sera salvo em " << filename << "." << endl;
		write_commands(filename);
	}

	try {
		// Lê o arquivo com o código
		string codigo = ler_codigo_de_arquivo(filename);

		cout << "===Arquivo lido===" << endl;
		cout << codigo << endl;
		cout << "==================" << endl;
		cout << endl;

		// Gera expressões a partir de um pré-processamento do código, separadas pelo ; e sem espaços em branco.
		vector<string> expressoes = parse_codigo(codigo);

		// Gera as instruções apartir das expressões.
		vector<string> instrucoes = gerar_instrucoes(expressoes);

		cout << "===Instruções geradas===" << endl;
		for (const auto &instrucao : instrucoes)
			cout << instrucao << endl;
		cout << "========================" << endl;
		cout << endl;

		// Escreve as instruções geradas em arquivo .hex
		string hex_filename = filename.substr(0, filename.find(".")).append(".hex");
		escrever_vetor_em_arquivo(instrucoes, hex_filename);
		cout << "Instruções gravadas em " << hex_filename << endl;
		cout << endl;


		char resposta;
		cout << "Quer realizar a execução passo a passo? (s/n): ";
		cin >> resposta;
		cin.ignore();

		if (std::tolower(resposta) == 's') {
			if (porta_COM.empty()) {
				cout << "Digite a porta COM em que o arduino está conectado: ";
				std::getline(cin, porta_COM);
			}

			cout << "Iniciando execução passo a passo: " << endl;
			for (const auto &instrucao : instrucoes) {
				cout << "Instrução sendo executada agora: " << instrucao << endl;

				// Montamos o comando.
				string comando = "envia.exe ";
				comando += porta_COM + " ";
				comando += "\"" + instrucao + "\"";

				// Enviamos para o arduino.
				std::system(comando.c_str());

				cout << "Pressione enter para ir para a próxima instrução." << endl;
				cin.ignore();
			}
			cout << "Fim da execução" << endl;
		}
	} catch (const std::invalid_argument &exception) {
		cout << exception.what() << endl;
		cout << "Terminando execução do programa" << endl;
	}

	return 0;
}

// Dadas as expressões em um vetor, essa função gera as intruções que serão executadas.
vector<string> gerar_instrucoes(const vector<string> &expressoes) {
	// Instruções geradas pelas expressões ficarão armazenadas nesse vetor.
	vector<string> instrucoes;

	// Só há dois "registradores" nessa máquina, e as instruções só se aplicam a eles.
	char A = '0', B = '0';

	string::size_type posicao = 0;
	for (const string &expressao : expressoes) {
		// IF Temporário... (ou não? :thinking:)
		if (expressao == "fim" || expressao == "inicio")
			continue;

		// Se tem sinal =, é uma declaração ou alteração de variável.
		if ((posicao = expressao.find('=')) != string::npos) {
			char valor = expressao.substr(posicao + 1).at(0); // Pega o valor, que está depois do sinal.

			// Checa quem deve receber esse valor, registrador A ou B.
			if (expressao.at(0) == 'A')
				A = valor;
			else
				B = valor;
		} else {
			// Como não tem sinal de igualdade, é um mnemônico de uma instrução.
			string instrucao;

			// Primeiro código hexa da instrução é o valor guardado em A.
			instrucao += A;
			// Segundo código hexa da instrução é o valor guardado em B.
			instrucao += B;

			// Último código hexa é o código da instrução.
			// Para pegá-lo, basta pegar a expressão e buscar seu valor no mapa definido globalmente.
			string mnemonico = expressao;
			try {
				instrucao += mnemonicos_to_hex.at(mnemonico);
			} catch (const std::out_of_range &) {
				// Nesse caso, o programa não terminará de executar e instruções não serão geradas.
				throw std::invalid_argument("Erro: O mnemônico \"" + mnemonico + "\" não existe");
			}

			// Instrução feita, coloca no vetor de instruções.
			instrucoes.push_back(instrucao);
		}
	}

	return instrucoes;
}

// Funciona como um filtro para as expressões de um código pego em um arquivo.
// Tira os espaços em branco e as separa de acordo com os tokens . ; e :
vector<string> parse_codigo(const string &codigo) {
	vector<string> expressoes_parsed;	

	// Tira os espaços em branco das expressões lidas no arquivo.
	string expressoes_sem_espaco;
	for (const auto &c: codigo) {
		if (c != ' ')
			expressoes_sem_espaco += c;
	}

	// Uma expressão termina em : ; e .
	// Na verdade, : só se aplica a inicio:
	// e . só se aplica a fim.
	// Todas as outras terminam em ;
	string expressao;
	for (const auto &c : expressoes_sem_espaco) {
		// Fim de expressão.
		if (c == ';' || c == ':' || c == '.') {
			// Coloca no vetor de expressoes.
			expressoes_parsed.push_back(expressao);
			expressao.clear();
		} else {
			// Adiciona o próximo caractere que faz parte dessa expressão.
			expressao += c;
		}
	}

	return expressoes_parsed;
}

// Faz os comandos que serão escritos no arquivo estarem corretos
// Caso seja esquecido um ponto e vírgula, por exemplo.
string uniform(const string &comando) {
	string nova;

	if (comando != "fim.") {
        	for (const auto &chara : comando) {
            		if(chara != ' ')
			    nova += chara;
        	}

        	if(nova[nova.length()-1] != ';')
			nova += ';';
    	} else {
	    nova = "fim.";
	}

    	return nova;
}

// No modo interativo, comandos são escritos em um arquivo
// que será posteriormente executado.
void write_commands(const string &fileName) {
    	std::ofstream arqW;
    	string comando;
    	arqW.open(fileName);
    
    	arqW << "inicio:" << endl;

    	cout << "Insira comandos:" << endl;
    	std::getline(cin,comando);
    	comando = uniform(comando);
    	while(comando != "fim."){
        	arqW << comando << endl;
        	getline(cin,comando);
        	comando = uniform(comando);
   	 }

    	arqW << "fim." << endl;
    	arqW.close();
}

// Lê todo o código de um arquivo e coloca ele em uma string.
string ler_codigo_de_arquivo(const string &fileName) {
    	ifstream arquivo(fileName);

	if (!arquivo.is_open()) {
		throw std::invalid_argument("O arquivo \"" + fileName + "\" não existe.");
	}

	string expressoes;

	string comando;
	getline(arquivo,comando);
    	while(!arquivo.eof()){
		expressoes += comando;
        	getline(arquivo, comando);
    	}
    	arquivo.close();

    	return expressoes;
}

// Escreve um vetor de strings em um arquivo.
void escrever_vetor_em_arquivo(const vector<string> &vetor, const string &filename) {
	ofstream arquivo(filename);

	for (const auto &str : vetor) {
		arquivo << str << endl;
	}

	arquivo.close();
}
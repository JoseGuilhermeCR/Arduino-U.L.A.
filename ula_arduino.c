int toInt(char);

// Funções representando cada instrução.
void zeroL();
void umL();
void An();
void Bn();
void AouB();
void AeB();
void AxorB();
void AnandB();
void AnorB();
void AxnorB();
void AnouB();
void AouBn();
void AneB();
void AeBn();
void AnouBn();
void AneBn();

// Um array de funções que aponta para as funções de instrução em ordem crescente (0 até F).
void (*instrucoes[16]) () = {
                              zeroL, umL, An, Bn,
                              AouB, AeB, AxorB, AnandB,
                              AnorB, AxnorB, AnouB, AouBn,
                              AneB, AeBn, AnouBn, AneBn
                            };

// Guarda a linha que vem do computador.
String linha = String("");

// Variáveis da ULA.
// Cada variável na verdade só gasta 4 bits de cada char. Seria possível usar apenas 2 variáveis
// Juntando duas variáveis no mesmo byte.
char A, B, OP, SAIDA;

// Saída em cada porta.
int saida_0 = 13;
int saida_1 = 12;
int saida_2 = 11;
int saida_3 = 10;

void setup() {
    // Inicia comunicação serial.
    Serial.begin(9600);

    // Ajusta os pinos em que os leds estão para modo OUTPUT.
    pinMode(saida_0, OUTPUT);
    pinMode(saida_1, OUTPUT);
    pinMode(saida_2, OUTPUT);
    pinMode(saida_3, OUTPUT);
}

void loop() {
    // Se houver dados para serem lidos.
    if(Serial.available() > 0) {
        // Pega a instrução.
        linha = Serial.readString();

        // Só feedback.
        Serial.print("\nRecebi da porta serial: ");
        Serial.print(linha);

        // Coloca os valores númericos dentro das variáveis.
        A = toInt(linha.charAt(0));
        B = toInt(linha.charAt(1));
        OP = toInt(linha.charAt(2));

	if (Serial.read() == '\n') {
		// Só feedback.
		Serial.print("\nA = ");
		Serial.print((int)A);
		Serial.print(" B = ");
		Serial.print((int)B);
		Serial.print(" OP = ");
		Serial.print((int)OP);

		// Chama a função que representa esta instrução.
		(*instrucoes[OP])();

		// Após chamada de função, mostra nos 4 leds os 4 primeiros bits da saída.
		digitalWrite(saida_0, SAIDA & 0x8);
		digitalWrite(saida_1, SAIDA & 0x4);
		digitalWrite(saida_2, SAIDA & 0x2);
		digitalWrite(saida_3, SAIDA & 0x1);

		// Só feedback.
		Serial.print("\nSaida (em binário) = ");
		Serial.print((bool)(SAIDA & 0x8));
		Serial.print((bool)(SAIDA & 0x4));
		Serial.print((bool)(SAIDA & 0x2));
		Serial.print((bool)(SAIDA & 0x1));
	}
    }
}

// Converte caracteres de '0' até 'F' para seus respectivos valores numéricos.
int toInt(char c){
    int resp = 0;

    if(c >= 65 && c <= 70){
        resp = c - '7';
    } else {
        resp = c - '0';
    }

    return resp;
}

// Zero Lógico
void zeroL() {
    SAIDA = 0x0;
}

// Um Lógico
void umL() {
    SAIDA = 0xF;
}

// A'
void An() {
    SAIDA = ~A & 0xF;
}

// B'
void Bn() {
    SAIDA = ~B & 0xF;
}

// A + B
void AouB() {
    SAIDA = A | B;
}

// A . B
void AeB() {
    SAIDA = A & B;
}

// A ⊕ B
void AxorB() {
    SAIDA = A ^ B;
}

// (A . B)'
void AnandB() {
    SAIDA = ~(A & B);
}

// (A + B)'
void AnorB() {
    SAIDA = ~(A | B);
}

// (A ⊕ B)'
void AxnorB() {
    SAIDA = ~(A ^ B);
}

// A' + B
void AnouB() {
    SAIDA = (~A) | B;
}

// A + B'
void AouBn() {
    SAIDA = A | (~B);
}

// A' . B
void AneB() {
    SAIDA = (~A) & B;
}

// A . B'
void AeBn() {
    SAIDA = A & (~B);
}

// A' + B'
void AnouBn() {
    SAIDA = (~A) | (~B);
}

// A' . B'
void AneBn() {
    SAIDA = (~A) & (~B); 
}
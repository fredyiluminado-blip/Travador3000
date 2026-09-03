#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FirebaseESP32.h>
#include "config.h"

// --- CONFIGURAÇÕES DE HARDWARE ---
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PIN_RELE_SOLENOIDE 18
#define ABRIR_TRAVA HIGH   
#define FECHAR_TRAVA LOW

// --- VARIÁVEIS DE CONTROLE E ESTADO ---
enum EstadoSuporte {
  DISPONIVEL,
  EM_USO
};

EstadoSuporte estadoAtual = DISPONIVEL;
String alunoAtual = "";
String turmaAtual = ""; // Nova variável para a turma
String qrCodeAtual = "";
String idViolao = "01"; 

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config_fb;

// --- FUNÇÕES DE DISPLAY ---
void atualizarTela(String linha1, String linha2, String linha3 = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(linha1);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println(linha2);
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println(linha3);
  display.display();
}

// --- FUNÇÃO DE VALIDAÇÃO COM TURMA ---
bool validarAlunoFirebase(String qrCode, String &nomeAluno, String &turmaAluno) {
  atualizarTela("ACESSO", "Verificando...", "");
  Serial.print("Consultando Firebase para ID: ");
  Serial.println(qrCode);
  
  String pathNome = "/alunos/" + qrCode + "/nome";
  String pathTurma = "/alunos/" + qrCode + "/turma";

  // Busca o Nome
  if (Firebase.getString(firebaseData, pathNome)) {
    nomeAluno = firebaseData.stringData();
    if (nomeAluno == "null" || nomeAluno == "") {
        Serial.println("ID nao encontrado no banco.");
        return false;
    }
    
    // Busca a Turma (se não tiver turma cadastrada, deixa em branco para não travar)
    if (Firebase.getString(firebaseData, pathTurma)) {
      turmaAluno = firebaseData.stringData();
      if (turmaAluno == "null") turmaAluno = "";
    }
    
    Serial.println("Aluno validado: " + nomeAluno + " (" + turmaAluno + ")");
    return true;
  } else {
    Serial.println("Erro de conexao com o Firebase.");
    return false; 
  }
}

// --- SETUP ---
void setupWiFi() {
  atualizarTela("Wi-Fi", "Conectando...", "");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setupFirebase() {
  atualizarTela("Firebase", "Conectando...", "");
  config_fb.host = FIREBASE_HOST;
  config_fb.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config_fb, &auth);
  Firebase.reconnectWiFi(true); 
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RELE_SOLENOIDE, OUTPUT);
  digitalWrite(PIN_RELE_SOLENOIDE, FECHAR_TRAVA); // Inicia trancado
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }
  
  setupWiFi();
  setupFirebase();
  
  atualizarTela("  TRAVADOR3000  ", "DISPONIVEL", "Aproxime a carteirinha");
  Serial.println("Sistema V7 Online e Trancado.");
}

void loop() {
  if (Serial.available() > 0) {
    String leitura = Serial.readStringUntil('\n');
    leitura.trim();
    if (leitura == "") return;

    Serial.println("\n--- NOVA LEITURA: " + leitura + " ---");

    switch(estadoAtual) {
      
      case DISPONIVEL: {
        String nomeAluno, turmaAluno;
        
        if (validarAlunoFirebase(leitura, nomeAluno, turmaAluno)) {
          alunoAtual = nomeAluno;
          turmaAtual = turmaAluno;
          qrCodeAtual = leitura;
          estadoAtual = EM_USO;
          
          // LÓGICA NOVA: Abre a trava e MANTÉM ABERTA
          digitalWrite(PIN_RELE_SOLENOIDE, ABRIR_TRAVA); 
          Serial.println("-> Solenoide ABERTA (Permanecera assim ate a devolucao)");
          
          // Atualiza o Display com NOME e TURMA
          String linha3 = turmaAtual != "" ? "Turma: " + turmaAtual : "Uso autorizado";
          atualizarTela(" VIOLAO LIBERADO ", alunoAtual, linha3);
          
          // Atualiza Firebase 
          Firebase.setString(firebaseData, "/violoes/" + idViolao + "/status", "EM_USO");
          Firebase.setString(firebaseData, "/violoes/" + idViolao + "/aluno_matricula", qrCodeAtual);
          Serial.println("-> Banco de dados: Status alterado para EM_USO.");
          
        } else {
          atualizarTela("ERRO DE ACESSO", "INVALIDO", "Carteirinha nao cadastrada");
          delay(3000);
          atualizarTela("  TRAVADOR3000  ", "DISPONIVEL", "Aproxime a carteirinha");
        }
        break;
      }
      
      case EM_USO: {
        // Aluno encosta a carteirinha para DEVOLVER e TRANCAR
        if (leitura == qrCodeAtual) {
          Serial.println("-> Validando devolucao...");
          
          // LÓGICA NOVA: Tranca a trava agora!
          digitalWrite(PIN_RELE_SOLENOIDE, FECHAR_TRAVA); 
          Serial.println("-> Solenoide TRANCADA.");
          
          // Atualiza Firebase CORRIGIDO
          if(Firebase.setString(firebaseData, "/violoes/" + idViolao + "/status", "DISPONIVEL")) {
             Serial.println("-> Banco de dados: Status alterado para DISPONIVEL.");
          } else {
             Serial.println("-> ERRO: Falha ao atualizar banco de dados!");
          }
          Firebase.setString(firebaseData, "/violoes/" + idViolao + "/aluno_matricula", "");

          // Limpa a memória
          alunoAtual = "";
          turmaAtual = "";
          qrCodeAtual = "";
          estadoAtual = DISPONIVEL;
          
          atualizarTela("DEVOLUCAO ACEITA", "Sucesso!", "Obrigado.");
          delay(3000);
          atualizarTela("  TRAVADOR3000  ", "DISPONIVEL", "Aproxime a carteirinha");
          
        } else {
          atualizarTela("ERRO DEVOLUCAO", "INCORRETO", "Use a mesma carteirinha!");
          delay(3000);
          // Volta a mostrar quem está com o violão e a turma
          String linha3 = turmaAtual != "" ? "Turma: " + turmaAtual : "";
          atualizarTela(" VIOLAO RETIRADO ", alunoAtual, linha3);
        }
        break;
      }
    }
  }
}
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- NOVA BIBLIOTECA PARA INTEGRAÇÃO ---
#include <FirebaseESP32.h>

// Inclui o arquivo de configuração local (NÃO enviado ao GitHub)
#include "config.h"

// --- CONFIGURAÇÕES DE HARDWARE ---
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PIN_RELE_SOLENOIDE 18
// Ajuste HIGH/LOW se a trava inverter na simulação
#define ABRIR_TRAVA HIGH   
#define FECHAR_TRAVA LOW

// --- VARIÁVEIS DE CONTROLE E ESTADO ---
enum EstadoSuporte {
  DISPONIVEL,
  EM_USO
};

EstadoSuporte estadoAtual = DISPONIVEL;
String alunoAtual = "";
String qrCodeAtual = "";
String idViolao = "01"; // ID deste suporte

// --- NOVAS VARIÁVEIS PARA FIREBASE ---
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

// --- FUNÇÃO DE VALIDAÇÃO REAL NO FIREBASE ---
bool validarAlunoFirebase(String qrCode, String &nomeAluno) {
  atualizarTela("ACESSO", "Verificando...", "");
  Serial.print("Consultando Firebase para ID: ");
  Serial.println(qrCode);
  
  // Caminho no JSON: /alunos/CÓDIGO_LIDO/nome
  String path = "/alunos/" + qrCode + "/nome";

  // Faz a requisição GET ao Firebase
  if (Firebase.getString(firebaseData, path)) {
    nomeAluno = firebaseData.stringData();
    if (nomeAluno == "null" || nomeAluno == "") {
        Serial.println("ID nao encontrado no banco.");
        return false;
    }
    Serial.println("Aluno validado: " + nomeAluno);
    return true;
  } else {
    Serial.print("Erro de conexao. Firebase responde: ");
    Serial.println(firebaseData.errorReason());
    return false; 
  }
}

// --- FUNÇÕES DE SETUP ---
void setupWiFi() {
  atualizarTela("Wi-Fi", "Conectando...", "");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());
}

void setupFirebase() {
  atualizarTela("Firebase", "Conectando...", "");
  config_fb.host = FIREBASE_HOST;
  config_fb.api_key = FIREBASE_AUTH;
  
  // Inicia a conexão
  Firebase.begin(&config_fb, &auth);
  Firebase.reconnectWiFi(true); // Garante reconexão automática
  Serial.println("Firebase estruturado");
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RELE_SOLENOIDE, OUTPUT);
  digitalWrite(PIN_RELE_SOLENOIDE, FECHAR_TRAVA);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha ao iniciar o OLED"));
    for(;;);
  }
  
  // Executa os setups de conexão
  setupWiFi();
  setupFirebase();
  
  atualizarTela("  TRAVADOR V7  ", "DISPONIVEL", "Aproxime a carteirinha");
  Serial.println("Sistema V7 Online e Trancado.");
}

void loop() {
  // Simulação de leitura de QR Code via Terminal Serial
  if (Serial.available() > 0) {
    String leitura = Serial.readStringUntil('\n');
    leitura.trim();
    
    if (leitura == "") return;

    Serial.print("QR Code Lido via Serial: ");
    Serial.println(leitura);

    switch(estadoAtual) {
      
      case DISPONIVEL: {
        String nomeAluno;
        // Tenta validar o aluno NO FIREBASE REAL
        if (validarAlunoFirebase(leitura, nomeAluno)) {
          alunoAtual = nomeAluno;
          qrCodeAtual = leitura;
          
          atualizarTela("ACESSO LIBERADO!", alunoAtual, "Retire o violao...");
          
          digitalWrite(PIN_RELE_SOLENOIDE, ABRIR_TRAVA); 
          Serial.println("Solenoide Aberta para retirada.");
          
          // --- ATUALIZAÇÃO NO FIREBASE ---
          String pathStatus = "/violoes/" + idViolao + "/status";
          String pathAluno = "/violoes/" + idViolao + "/aluno_matricula";
          
          Firebase.setString(firebaseData, pathStatus, "EM_USO");
          Firebase.setString(firebaseData, pathAluno, qrCodeAtual);
          Serial.println("Firebase atualizado: Violao em uso.");
          
          delay(5000); // Tempo para simular a retirada
          
          digitalWrite(PIN_RELE_SOLENOIDE, FECHAR_TRAVA); 
          Serial.println("Solenoide Trancada.");
          
          estadoAtual = EM_USO;
          atualizarTela(" VIOLAO RETIRADO ", "EM USO", "Com: " + alunoAtual);
        } else {
          atualizarTela("ERRO DE ACESSO", "INVALIDO", "Carteirinha nao cadastrada");
          Serial.print("Display ERRO: INVALIDO: ");
          Serial.println(leitura);
          delay(3000);
          atualizarTela("  TRAVADOR V7  ", "DISPONIVEL", "Aproxime a carteirinha");
        }
        break;
      }
      
      case EM_USO: {
        // Valida se o aluno que está devolvendo é o MESMO que pegou
        if (leitura == qrCodeAtual) {
          atualizarTela("DEVOLUCAO ACEITA", alunoAtual, "Insira o violao...");
          
          digitalWrite(PIN_RELE_SOLENOIDE, ABRIR_TRAVA); 
          Serial.println("Solenoide Aberta para devolucao.");
          
          delay(5000); // Tempo para simular o encaixe
          
          digitalWrite(PIN_RELE_SOLENOIDE, FECHAR_TRAVA); 
          Serial.println("Solenoide Trancada.");
          
          // --- ATUALIZAÇÃO NO FIREBASE ---
          String pathStatus = "/violoes/" + idViolao + "/status";
          String pathAluno = "/violoes/" + idViolao + "/aluno_matricula";
          
          Firebase.setString(firebaseData, pathStatus, "DISPONIVEL");
          Firebase.setString(firebaseData, pathAluno, "");
          Serial.println("Firebase atualizado: Violao devolvido.");

          alunoAtual = "";
          qrCodeAtual = "";
          estadoAtual = DISPONIVEL;
          
          atualizarTela("  TRAVADOR V7  ", "DISPONIVEL", "Aproxime a carteirinha");
        } else {
          atualizarTela("ERRO DEVOLUCAO", "INCORRETO", "Use a mesma carteirinha!");
          Serial.print("Display ERRO DEVOLUCAO: INCORRETO: ");
          Serial.println(leitura);
          delay(3000);
          atualizarTela(" VIOLAO RETIRADO ", "EM USO", "Com: " + alunoAtual);
        }
        break;
      }
    }
  }
}
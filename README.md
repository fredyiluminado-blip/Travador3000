# Travador3000 - Sistema de Trava Inteligente para Violões
(readme feito pelo Gemini KKKKKKKKKKKKKKKKKKKKK)

## Descrição do Projeto

O **Travador3000** é um sistema de monitoramento e controle de empréstimo de violões para o ambiente escolar. O projeto utiliza a carteirinha de estudante existente como método de autenticação, oferecendo segurança ao patrimônio da escola e adicionando uma funcionalidade útil e moderna à identificação dos alunos.

Este projeto busca dar segurança ao monitoramento dos instrumentos expostos na escola, automatizando o processo de retirada e devolução.

## Funcionalidades Principais

-   ✅ **Autenticação via QR Code**: Utiliza a carteirinha de estudante existente para identificar o aluno de forma única.
-   ✅ **Trava Eletrônica Automática**: Bloqueio e desbloqueio automático de solenoide de segurança ao validar a carteirinha.
-   ✅ **Monitoramento em Tempo Real**: Integração com Firebase Realtime Database para rastreamento de uso e histórico de empréstimos.
-   ✅ **Feedback Visual**: Display no módulo exibe Nome, Turma e Status do aluno e violão.
-   ✅ **Fluxo Seguro de Empréstimo**: Requer autenticação tanto para a retirada quanto para a devolução, garantindo a responsabilização do aluno.

## Hardware Necessário (PoC Funcional)

Abaixo estão listados os componentes necessários para construir a Prova de Conceito funcional do suporte:

-   *Microcontrolador ESP32 (Wi-Fi)*: O cérebro do sistema.
-   *Leitor de QR Code / Câmera (ex: ESP32-CAM ou módulo dedicado)*: Para ler a carteirinha.
-   *Trava Elétrica de Solenoide (ex: 12V)*: Para prender o violão fisicamente.
-   *Módulo Relé*: Para controlar a trava.
-   *Display OLED/LCD I2C*: Para feedback visual (como Nome e Turma).
-   *Fonte de alimentação adequada (ex: 12V para solenoide + regulador para ESP32)*.
-   *Peças para o suporte físico*: Madeira para a base, hastes metálicas e caixa para os eletrônicos (conforme fotos anexadas).

## Software Necessário

-   *Arduino IDE (com suporte a placas ESP32 e bibliotecas I2C/Firebase ESP32 Cliente)*.
-   *Firebase console account (para configuração do banco de dados Realtime Database)*.
-   *(Opcional) Blender (para modelagem com Scripts em Python)*.

## Organização do Repositório (Sugestão)

-   📂 `codigo_arduino/`: Contém o código principal (`ViolaoSmart.ino`).
-   📂 `codigo_blender/`: Pasta opcional com scripts de modelagem Python (como `Suporte_Violao_V7.py`).
-   📂 `banco_dados/`: Pasta opcional com arquivo JSON de estrutura (`estrutura_database.json`) e guia de setup (`setup_firebase.md`).
-   📂 `hardware/`: Pasta para diagramas e fotos do protótipo.
-   📄 `README.md`: Este arquivo de documentação.

## Fotos do Protótipo (Modelo 3D do Suporte)

Abaixo estão as imagens do modelo do protótipo físico (ou modelo 3D) do suporte de violão desenvolvidos até agora. Estas imagens mostram o design do suporte físico, mas não incluem o hardware de QR Code necessário para a funcionalidade:

<img width="537" height="474" alt="image" src="https://github.com/user-attachments/assets/97c65cca-62d0-472d-a838-f25bd4cdcdfa" />
<img width="614" height="699" alt="image" src="https://github.com/user-attachments/assets/bb6f08ad-56f7-4bb6-8a80-9e21f4e16841" />
<img width="703" height="625" alt="image" src="https://github.com/user-attachments/assets/48292bc2-9755-4f86-9925-198340bfa602" />
<img width="1089" height="697" alt="image" src="https://github.com/user-attachments/assets/92eb88d8-f9e5-4392-9128-a72a9db2c7df" />


## Autor

Projeto desenvolvido por Toshi (Fredy S. D. Luz) e zKuri (Felipe L. D. Oliveira).

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

// Protótipos das funções da SDK do PS1
void ResetGraph(int mode);
void SetVideoMode(int mode);
void SetDefDrawEnv(void *env, int x, int y, int w, int h);
void SetDefDispEnv(void *env, int x, int y, int w, int h);
void PutDrawEnv(void *env);
void PutDispEnv(void *env);
void InitPAD(void *buf1, int len1, void *buf2, int len2);
void StartPAD(void);
void FntLoad(int tx, int ty);
int FntOpen(int x, int y, int w, int h, int is_bg, int max_char);
int FntFlush(int id);
int FntPrint(int id, const char *fmt, ...);
void DrawSync(int mode);
int VSync(int mode);
int rsin(int a);
int rcos(int a);

// Mapeamento de Botões do PS1
#define PADLup      (1<<12)
#define PADLdown    (1<<14)
#define PADLleft    (1<<15)
#define PADLright   (1<<13)
#define PADRup      (1<<4)   // Triângulo
#define PADRdown    (1<<6)   // Xis
#define PADRleft    (1<<7)   // Quadrado
#define PADRright   (1<<5)   // Círculo

#define VIDEO_LARGURA 320
#define VIDEO_ALTURA  240
#define MAPA_LARGURA 16
#define MAPA_ALTURA 16
#define TOTAL_CADERNOS 4 

unsigned char db_draw[64];
unsigned char db_disp[64];
unsigned char buffer_controle1[34]; 
unsigned char buffer_controle2[34]; 

int idFntP1, idFntP2;

int idioma = 0; 
int telaAtual = 0; 
int posicaoMenu = 0;
int modoJogadores = 0; // 0 = 1 Player, 1 = Tela Dividida

// Dados do Player 1
int playerX = 8 << 12; int playerY = 4 << 12; int playerA = 0; 
int stamina = 100; int estaCorrendo = 0;

// Dados do Player 2
int player2X = 8 << 12; int player2Y = 5 << 12; int player2A = 0;
int stamina2 = 100; int estaCorrendo2 = 0;

int cadernosColetados = 0; 
int frameContador = 0; int minutos = 0; int segundos = 0;
int temBsoda = 0; int temChocolate = 0;

int baldiX = 2 << 12; int baldiY = 2 << 12; int baldiVel = 0;

// Variáveis do Minigame de Matemática e Sistema de Fila
int mathNum1 = 0; int mathNum2 = 0; int mathRespostaCorreta = 0;
int mathOpcaoSelecionada = 0; int mathOpcoes[3];
int perguntasPendentes = 0; 
int quemPegouO_Caderno = 1; 

char* textosMenu[3][4] = {
    {"Iniciar Jogo", "Configuracoes", "Creditos", "Sair Jogo"},
    {"Start Game", "Options", "Credits", "Exit Game"},
    {"Iniciar Juego", "Configuraciones", "Creditos", "Salir Juego"}
};

char* textosOptions[3][3] = {
    {"Idioma: Portugues", "Modo: Alternar", "Voltar"},
    {"Language: English", "Mode: Toggle", "Back"},
    {"Idioma: Espanol", "Modo: Alternar", "Volver"}
};

char mapa[MAPA_ALTURA][MAPA_LARGURA] = {
    "1111111111111111", 
    "1C000100001000H1", 
    "1000010000100001", 
    "1110110000110111",
    "1000000000000001", 
    "1011111001111101", 
    "1010001001000101", 
    "10000B0000000001",
    "1010001001000101", 
    "1011111001111101", 
    "1000000000000001", 
    "1110110000110111",
    "1000010000100001", 
    "1C000100001000C1", 
    "1111111111111111"
};

void InicializarSistemaPS1() {
    SetVideoMode(0); 
    ResetGraph(0);
    SetDefDrawEnv(db_draw, 0, 0, VIDEO_LARGURA, VIDEO_ALTURA);
    SetDefDispEnv(db_disp, 0, 0, VIDEO_LARGURA, VIDEO_ALTURA);
    PutDrawEnv(db_draw); 
    PutDispEnv(db_disp);
    
    InitPAD(buffer_controle1, 34, buffer_controle2, 34); 
    StartPAD();
    FntLoad(960, 256); 
    
    idFntP1 = FntOpen(8, 8, 304, 104, 0, 512);
    idFntP2 = FntOpen(8, 120, 304, 104, 0, 512);
}

void InverterBuffersPS1() {
    DrawSync(0); 
    VSync(0); 
}

void ResetarPartida() {
    playerX = 8 << 12; playerY = 4 << 12; playerA = 0;
    player2X = 8 << 12; player2Y = 5 << 12; player2A = 0;
    baldiX = 2 << 12; baldiY = 2 << 12;
    cadernosColetados = 0; stamina = 100; stamina2 = 100;
    minutos = 0; segundos = 0; frameContador = 0;
    temBsoda = 0; temChocolate = 0;
    perguntasPendentes = 0;
    
    mapa[1][1] = 'C'; mapa[1][14] = 'H'; mapa[7][5] = 'B'; mapa[13][1] = 'C'; mapa[13][14] = 'C';
}

void GerarPerguntaMatematica(int jogadorID) {
    quemPegouO_Caderno = jogadorID;
    mathNum1 = ((frameContador + jogadorID) % 9) + 1;
    mathNum2 = (((frameContador * 3) + (jogadorID * 2)) % 8) + 1;
    mathRespostaCorreta = mathNum1 + mathNum2;
    
    int ordenacao = (frameContador + jogadorID) % 3;
    if (ordenacao == 0) {
        mathOpcoes[0] = mathRespostaCorreta; mathOpcoes[1] = mathRespostaCorreta + 2; mathOpcoes[2] = mathRespostaCorreta - 1;
    } else if (ordenacao == 1) {
        mathOpcoes[0] = mathRespostaCorreta - 2; mathOpcoes[1] = mathRespostaCorreta; mathOpcoes[2] = mathRespostaCorreta + 1;
    } else {
        mathOpcoes[0] = mathRespostaCorreta + 1; mathOpcoes[1] = mathRespostaCorreta - 1; mathOpcoes[2] = mathRespostaCorreta;
    }
    mathOpcaoSelecionada = 0;
}

int main() {
    int direcionalTrava = 0; int botaoTrava = 0;
    InicializarSistemaPS1();
    
    while (1) {
        FntFlush(idFntP1);
        if(modoJogadores == 1) FntFlush(idFntP2); 
        
        unsigned short pad1 = ~((unsigned short*)(buffer_controle1))[1];
        unsigned short pad2 = ~((unsigned short*)(buffer_controle2))[1];
        
        char* textoModoAtivo;
        if(modoJogadores == 0) {
            textoModoAtivo = (idioma == 0) ? "Modo: 1 Jogador" : ((idioma == 1) ? "Mode: 1 Player" : "Modo: 1 Jugador");
        } else {
            textoModoAtivo = (idioma == 0) ? "Modo: Tela Dividida" : ((idioma == 1) ? "Mode: Split-Screen" : "Modo: Pantalla Dividida");
        }
        textosOptions[idioma][1] = textoModoAtivo;

        // TELAS 0 e 1: MENU E OPTIONS
        if (telaAtual == 0 || telaAtual == 1) {
            FntPrint(idFntP1, "     BALDI'S BASICS - PSX MENU     \n");
            FntPrint(idFntP1, "===================================\n\n");
            int limite = (telaAtual == 0) ? 4 : 3;
            int i;
            if (telaAtual == 0) {
                for(i=0; i<4; i++) FntPrint(idFntP1, i==posicaoMenu ? " -> [ %s ] <-\n" : "    %s\n", textosMenu[idioma][i]);
            } else {
                for(i=0; i<3; i++) FntPrint(idFntP1, i==posicaoMenu ? " -> [ %s ] <-\n" : "    %s\n", textosOptions[idioma][i]);
            }
            
            if (!direcionalTrava) {
                if (pad1 & PADLup) { posicaoMenu = (posicaoMenu - 1 + limite) % limite; direcionalTrava = 1; }
                if (pad1 & PADLdown) { posicaoMenu = (posicaoMenu + 1) % limite; direcionalTrava = 1; }
            }
            if (!(pad1 & (PADLup | PADLdown))) direcionalTrava = 0;
            
            if (!botaoTrava && (pad1 & PADRdown)) { 
                botaoTrava = 1;
                if (telaAtual == 0) {
                    if (posicaoMenu == 0) { telaAtual = 3; ResetarPartida(); }
                    else if (posicaoMenu == 1) { telaAtual = 1; posicaoMenu = 0; }
                    else if (posicaoMenu == 2) telaAtual = 2;
                    else if (posicaoMenu == 3) ResetGraph(3);
                } else {
                    if (posicaoMenu == 0) idioma = (idioma + 1) % 3;
                    else if (posicaoMenu == 1) modoJogadores = (modoJogadores + 1) % 2;
                    else { telaAtual = 0; posicaoMenu = 1; }
                }
            }
            if (!botaoTrava && (pad1 & PADRup)) { botaoTrava = 1; if (telaAtual == 1) { telaAtual = 0; posicaoMenu = 0; } }
            if (!(pad1 & (PADRdown | PADRup))) botaoTrava = 0;
        }
        
        // TELA 2: CRÉDITOS
        else if (telaAtual == 2) {
            FntPrint(idFntP1, "     BALDI'S BASICS - CREDITS      \n");
            FntPrint(idFntP1, "===================================\n\n");
            FntPrint(idFntP1, idioma == 0 ? " Criado para rodar no Game Stick\n" : " Made for Game Stick PS1!\n");
            FntPrint(idFntP1, "\n Pressione TRIANGULO para voltar...");
            if (pad1 & PADRup) { telaAtual = 0; posicaoMenu = 2; }
        }
        
        // TELA 3: GAMEPLAY ATIVA
        else if (telaAtual == 3) {
            frameContador++;
            if (frameContador >= 60) { frameContador = 0; segundos++; if (segundos >= 60) { segundos = 0; minutos++; } }
            
            int dist1 = (playerX >> 12) - (baldiX >> 12) + (playerY >> 12) - (baldiY >> 12);
            int dist2 = (player2X >> 12) - (baldiX >> 12) + (player2Y >> 12) - (baldiY >> 12);
            if(dist1 < 0) dist1 = -dist1; if(dist2 < 0) dist2 = -dist2;
            int alvoX = playerX; int alvoY = playerY;
            if(modoJogadores == 1 && dist2 < dist1) { alvoX = player2X; alvoY = player2Y; }

            baldiVel++;
            if (baldiVel >= (8 - cadernosColetados)) {
                baldiVel = 0;
                if (baldiX < alvoX && mapa[baldiY >> 12][(baldiX + (1 << 12)) >> 12] != '1') baldiX += (1 << 12);
                else if (baldiX > alvoX && mapa[baldiY >> 12][(baldiX - (1 << 12)) >> 12] != '1') baldiX -= (1 << 12);
                if (baldiY < alvoY && mapa[(baldiY + (1 << 12)) >> 12][baldiX >> 12] != '1') baldiY += (1 << 12);
                else if (baldiY > alvoY && mapa[(baldiY - (1 << 12)) >> 12][baldiX >> 12] != '1') baldiY -= (1 << 12);
            }
            
            if (((playerX >> 12) == (baldiX >> 12) && (playerY >> 12) == (baldiY >> 12)) ||
                (modoJogadores == 1 && (player2X >> 12) == (baldiX >> 12) && (player2Y >> 12) == (baldiY >> 12))) { 
                telaAtual = 4; 
            }
            
            if (cadernosColetados >= TOTAL_CADERNOS && ((playerX >> 12) <= 1 || (playerX >> 12) >= 14 || (playerY >> 12) <= 1 || (playerY >> 12) >= 14)) { telaAtual = 5; }
            
            // CONTROLES JOGADOR 1
            if ((pad1 & PADRright) && stamina > 5) { estaCorrendo = 1; stamina -= 2; } 
            else { estaCorrendo = 0; if (stamina < 100) stamina++; }
            int velFisica = estaCorrendo ? 256 : 128; 
            if (pad1 & PADLleft)  playerA = (playerA - 64) & 4095;
            if (pad1 & PADLright) playerA = (playerA + 64) & 4095;
            if (pad1 & PADLup) {

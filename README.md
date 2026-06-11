# Baldis.psx-raycasting

Um motor de renderização pseudo-3D (Raycasting) escrito em **Linguagem C**, projetado especificamente com foco na estética e limitações de hardware do **PlayStation 1 (PSX)**.

## 📱 Diferencial do Projeto
O desenvolvimento deste projeto possui um grande desafio técnico: ele está sendo codificado e gerenciado **inteiramente através de um dispositivo móvel (celular)**.

## ⚙️ Arquitetura e Limitações (PSX)
Diferente dos motores de raycasting tradicionais modernos, este motor busca se adaptar às restrições clássicas do hardware do PS1:
* **Matemática de Ponto Fixo:** Substituição de operações pesadas com `float` para otimizar o processamento na CPU do console.
* **Comunicação com a GPU:** Estruturação futura voltada para as bibliotecas de vídeo nativas do PS1 (como a `libgpu.h`).

## 🛠️ Status Atual
O projeto está em sua fase inicial de desenvolvimento lógico e estruturação de algoritmos de colisão e varredura de raios.

---
*Criado por Bernardo1432*

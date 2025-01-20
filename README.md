## Babel
Modelagem concorrente de sistema de comunicação militar seguro utilizando a cifra de Vigenère e mecanismos de sincronização concorrente da biblioteca Pthreads. O sistema é composto por quatro entidades:

* **General:** Gera ordens militares aleatórias.
* **Tenente:** Codifica as ordens usando a cifra de Vigenère.
* **Pombo:** Transporta as mensagens codificadas.
* **Cabo:** Decodifica as mensagens e executa as ordens.

A cifra de Vigenère foi implementada de forma otimizada por meio de arimética modular, sem a necessidade de armazenar a tradicional tábula reta.

**Instruções para Execução:**

1. **Compilar o código:**
   ```bash
   gcc guerra.c -o guerra -pthread
   ```

2. **Executar o programa:**
   ```bash
   ./guerra
   ```

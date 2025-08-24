# Como Usar o BarrierLayer

Este guia detalha como usar o BarrierLayer para rodar executáveis do Windows, aplicando uma camada de compatibilidade para sistemas anti-cheat.

## O Conceito: Duas Camadas

O BarrierLayer funciona com duas camadas principais que agora são combinadas no script de inicialização:

1.  **A Sandbox (`bwrap`):** Cria um "palco" isolado para o jogo rodar, controlando quais arquivos ele pode ver. Isso garante que o Wine funcione em um ambiente limpo e previsível.
2.  **A Camada Anti-Cheat (`barrierlayer_hook.so`):** Esta é a "mágica" do projeto. É uma biblioteca que é pré-carregada (`LD_PRELOAD`) junto com o jogo. Ela intercepta chamadas de sistema sensíveis que os anti-cheats usam, respondendo de uma maneira que os faz pensar que estão rodando em um ambiente Windows nativo.

O script `run_with_barrierlayer.sh` automatiza a criação da sandbox e a injeção da camada anti-cheat.

## Passo 1: Pré-requisitos

1.  **Compile o Projeto:** A camada anti-cheat (`barrierlayer_hook.so`) precisa ser compilada. Se você ainda não o fez, rode o comando `make` na raiz do projeto.
    ```bash
    make all
    ```
2.  **Instale as Dependências:** Certifique-se de ter o `wine` e o `bubblewrap` instalados.
    ```bash
    # Exemplo para Debian/Ubuntu
    sudo apt-get update
    sudo apt-get install wine bubblewrap
    ```

## Passo 2: Rodando um Executável

Para rodar um jogo ou aplicativo com a proteção do BarrierLayer, passe o caminho do arquivo `.exe` como argumento para o script `scripts/run_with_barrierlayer.sh`.

**Exemplo de Uso:**

```bash
# Para rodar um jogo com anti-cheat
./scripts/run_with_barrierlayer.sh /path/to/your/game.exe
```

### O que o script faz:
1.  Verifica se a biblioteca `barrierlayer_hook.so` existe.
2.  Copia temporariamente o seu `.exe` para dentro do ambiente `fake_c_drive`.
3.  Inicia la sandbox segura com `bwrap`.
4.  Configura o `LD_PRELOAD` para injetar a biblioteca de hook, ativando a camada anti-cheat.
5.  Executa o `.exe` com o Wine dentro da sandbox protegida.
6.  Remove a cópia do `.exe` após a finalização.

## (Opcional) Integração com o Steam

Você pode usar este script para rodar jogos da Steam com anti-cheat.

1.  Na sua biblioteca Steam, clique com o botão direito no jogo e vá em **Propriedades...**
2.  Na aba **GERAL**, encontre as **OPÇÕES DE INICIALIZAÇÃO**.
3.  Cole o seguinte comando, **substituindo `/caminho/absoluto/para/BarrierLayer` pelo caminho real** da pasta do projeto.

    ```
    /caminho/absoluto/para/BarrierLayer/scripts/run_with_barrierlayer.sh %command%
    ```
4.  Feche as propriedades e inicie o jogo. O script irá interceptar o comando e rodá-lo dentro da sandbox com a proteção anti-cheat ativada.

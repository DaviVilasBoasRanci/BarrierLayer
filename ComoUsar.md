# Como Usar o BarrierLayer (Fase de Análise)

Este documento descreve como utilizar o BarrierLayer em seu estado atual. Nesta fase, o projeto funciona como uma **ferramenta de diagnóstico e análise**, e não ainda como uma camada de compatibilidade completa. O objetivo é interceptar e registrar as ações que os sistemas anti-cheat (EAC, BattlEye) realizam para entendermos seu comportamento.

---

## Como Funciona?

O BarrierLayer utiliza o mecanismo `LD_PRELOAD` do Linux. Ele funciona da seguinte forma:

1.  **Injeção:** Uma biblioteca compartilhada (`barrierlayer_hook.so`) é "pré-carregada" antes que o jogo e o anti-cheat iniciem.
2.  **Interceptação:** Nossa biblioteca contém implementações falsas de funções críticas da API do Windows (como `CreateFileW`, `OpenProcess`, etc.). Quando o anti-cheat tenta chamar uma dessas funções, ele chama a nossa versão primeiro.
3.  **Registro (Logging):** Nossa função interceptadora registra a chamada e seus parâmetros (ex: o arquivo que tentou abrir, o processo que tentou inspecionar) em um arquivo de log chamado `barrierlayer_activity.log`.
4.  **Passagem (Placeholder):** Por enquanto, após registrar a chamada, a ferramenta simplesmente tenta passar a chamada para a função original (se existir no ambiente Wine/Proton). O objetivo principal agora é **coletar informações**.

O arquivo `barrierlayer_activity.log` é o resultado mais importante desta fase. Ele é o nosso guia para saber o que precisa ser traduzido ou emulado.

---

## Passo a Passo para Uso (com Steam/Proton)

Siga estes passos para gerar um log de atividades de um jogo.

### 1. Compilar o Projeto

Antes de tudo, você precisa compilar a biblioteca de hooks. Abra um terminal na pasta do projeto e execute:

```bash
make build
```

Isso criará a biblioteca `bin/barrierlayer_hook.so`.

### 2. Obter o Comando de Inicialização

Para facilitar o uso, criamos um comando que gera as instruções exatas para você. No terminal, execute:

```bash
make usage
```

A saída será algo como:

```
--- Instruções de Uso (Steam/Proton) ---
1. Compile o projeto com: make build
2. Vá para as propriedades do seu jogo na Steam.
3. Em 'Opções de Inicialização', cole o seguinte comando:
   ENABLE_BARRIERLAYER=1 /home/davivbrdev/BarrierLayer/scripts/run_with_barrierlayer.sh %command%

O log de atividades será salvo em: /home/davivbrdev/BarrierLayer/barrierlayer_activity.log
```

### 3. Configurar a Steam

- Abra a Steam.
- Vá para a sua **Biblioteca**.
- Clique com o botão direito no jogo que você quer testar (ex: um jogo com Easy Anti-Cheat ou BattlEye).
- Selecione **Propriedades...**.
- Na aba **Geral**, encontre o campo **Opções de Inicialização**.
- Copie e cole o comando completo que você obteve no passo anterior.

### 4. Executar e Gerar o Log

Feche a janela de propriedades e inicie o jogo normalmente pela Steam. Jogue por alguns minutos, especialmente em momentos em que o anti-cheat estaria mais ativo (como ao entrar em um lobby ou partida online).

O script `run_with_barrierlayer.sh` ativará o hook automaticamente.

### 5. Analisar os Resultados

Após fechar o jogo, um arquivo chamado `barrierlayer_activity.log` terá sido criado na pasta raiz do projeto. Este arquivo contém a lista de todas as operações que o BarrierLayer interceptou.

---

## Próximos Passos

Com o log em mãos, o próximo grande passo do projeto é analisar essas informações e começar a implementar a lógica de **tradução** dentro de cada função de hook, convertendo as chamadas do Windows para suas equivalentes no Linux e tratando os casos em que o anti-cheat espera um comportamento específico.

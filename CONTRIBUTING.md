# Guia de Contribuição para o BarrierLayer

Primeiramente, obrigado pelo seu interesse em contribuir com o BarrierLayer! Toda ajuda é bem-vinda. Este documento fornece diretrizes para quem deseja contribuir com o projeto.

## Como Contribuir

Existem várias maneiras de contribuir, desde relatar bugs até escrever código.

### Relatando Bugs

Se você encontrar um bug, por favor, verifique se ele já não foi relatado na seção [Issues](https://github.com/seu-usuario/BarrierLayer/issues) do GitHub.

Se não houver um relatório existente, crie uma nova issue, fornecendo as seguintes informações:

- **Versão do BarrierLayer:** (ex: v2.0)
- **Distribuição Linux e Versão do Kernel:** (ex: Ubuntu 22.04, kernel 5.15)
- **Jogo/Aplicação afetada:**
- **Versão do Proton/Wine (se aplicável):**
- **Descrição clara e concisa do bug.**
- **Passos para reproduzir o bug.**
- **Logs relevantes:** (use `export BARRIERLAYER_LOG_LEVEL=0` para logs detalhados)

### Sugerindo Melhorias e Recursos

Se você tem uma ideia para uma nova funcionalidade ou uma melhoria, sinta-se à vontade para abrir uma issue para discutir sua ideia. Descreva o recurso proposto e por que ele seria útil para o projeto.

### Processo de Pull Request (PR)

1.  **Faça o Fork do Repositório:** Crie um fork do projeto para a sua conta do GitHub.
2.  **Crie uma Branch:** Crie uma branch para a sua contribuição (`git checkout -b minha-feature`).
3.  **Desenvolva e Teste:** Faça as suas alterações no código. Certifique-se de que o código segue os padrões do projeto e que os testes (se aplicável) passam.
    -   Compile o projeto com `make`.
    -   Execute os testes com `make test`.
4.  **Faça o Commit das Suas Alterações:** Use mensagens de commit claras e descritivas (`git commit -m "feat: Adiciona nova feature X"`).
5.  **Envie para o Seu Fork:** Faça o push da sua branch para o seu fork (`git push origin minha-feature`).
6.  **Abra um Pull Request:** Abra um PR do seu fork para o repositório principal do BarrierLayer. Forneça uma descrição clara das alterações no PR.

## Padrões de Código

-   Mantenha o estilo de código existente.
-   Comente o código quando a lógica for complexa.
-   Certifique-se de que o código compila sem warnings.

Obrigado novamente por sua contribuição!

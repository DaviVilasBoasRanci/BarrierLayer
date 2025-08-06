#!/bin/bash
# Instala o script run_with_barrierlayer.sh no PATH do usuário
mkdir -p "$HOME/.local/bin"
cp "$(dirname "$0")/scripts/run_with_barrierlayer.sh" "$HOME/.local/bin/run_with_barrierlayer.sh"
chmod +x "$HOME/.local/bin/run_with_barrierlayer.sh"
echo "Script instalado em $HOME/.local/bin/run_with_barrierlayer.sh"
echo "Adicione $HOME/.local/bin ao seu PATH se ainda não estiver."

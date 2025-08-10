#!/bin/bash

# BarrierLayer - Script de Demonstração dos Hooks
# Este script demonstra o funcionamento dos hooks implementados

echo "=== BarrierLayer - Demonstração dos Hooks ==="
echo "Total de hooks implementados: 195+"
echo ""

# Verificar se os binários foram compilados
if [ ! -f "bin/barrierlayer_hook.so" ]; then
    echo "❌ Erro: barrierlayer_hook.so não encontrado. Execute 'make build' primeiro."
    exit 1
fi

if [ ! -f "bin/test_runner" ]; then
    echo "⚠️  Compilando test_runner..."
    make bin/test_runner
fi

echo "✅ Binários encontrados!"
echo ""

# Limpar log anterior
LOG_FILE="barrierlayer_activity.log"
if [ -f "$LOG_FILE" ]; then
    rm "$LOG_FILE"
fi

echo "🚀 Executando demonstração com LD_PRELOAD..."
echo "📝 Logs serão salvos em: $LOG_FILE"
echo ""

# Executar teste com hooks
LD_PRELOAD=./bin/barrierlayer_hook.so ./bin/test_runner

echo ""
echo "📊 Resultados da demonstração:"
echo ""

if [ -f "$LOG_FILE" ]; then
    echo "✅ Log de atividades gerado com sucesso!"
    echo "📈 Número de hooks interceptados: $(wc -l < "$LOG_FILE")"
    echo ""
    echo "🔍 Primeiras 10 interceptações:"
    head -10 "$LOG_FILE"
    echo ""
    echo "🔍 Últimas 5 interceptações:"
    tail -5 "$LOG_FILE"
    echo ""
    
    # Estatísticas por categoria
    echo "📊 Estatísticas por categoria de hooks:"
    echo "🌐 Rede (NET):      $(grep -c "NET:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "🖥️  Hardware (HW):   $(grep -c "HW:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "🔐 Crypto (CRYPTO): $(grep -c "CRYPTO:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "🧵 Threading (THR): $(grep -c "THR:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "💾 Memória (MEM):   $(grep -c "MEM:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "⚙️  Kernel (KERN):   $(grep -c "KERN:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "🔧 Serviços (SVC):  $(grep -c "SVC:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "📊 WMI (WMI):       $(grep -c "WMI:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "🐛 Debug (DBG):     $(grep -c "DBG:" "$LOG_FILE" 2>/dev/null || echo 0)"
    echo "📁 Outros:          $(grep -c "HOOK:" "$LOG_FILE" 2>/dev/null || echo 0)"
    
else
    echo "⚠️  Nenhum log foi gerado. Verifique se os hooks estão funcionando corretamente."
fi

echo ""
echo "🎯 Demonstração concluída!"
echo ""
echo "💡 Para usar com jogos:"
echo "   ENABLE_BARRIERLAYER=1 run_with_barrierlayer.sh %command%"
echo ""
echo "🖥️  Para usar a interface gráfica:"
echo "   ./bin/barrierlayer_gui"
echo ""
echo "📚 Para mais informações, consulte HOOKS_DOCUMENTATION.md"
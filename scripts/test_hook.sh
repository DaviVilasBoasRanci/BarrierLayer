#!/bin/bash

# Este script executa o alvo de teste do Makefile, que lida com
# a compilação e a execução do teste com LD_PRELOAD.

set -e

make test
# EP - PCS3616 (Sistemas de Programação): Montador e Desmontador MVN

Este repositório contém o Exercício-Programa (EP) da disciplina PCS3616 - Sistemas de Programação.

O objetivo do projeto é implementar, em C, duas ferramentas para a máquina MVN:

- `mnem2op`: montador (`.asm` -> `.mvn`)
- `op2mnem`: desmontador (`.mvn` -> `.asm`)

As instruções completas do EP estão em `instructions.txt`.

## Objetivo do EP

Segundo o enunciado, os programas devem:

- usar os mesmos opcodes e mnemônicos da MVN;
- trabalhar com mnemônicos em maiúsculo;
- seguir a sintaxe dos arquivos `.asm` e `.mvn` da disciplina;
- assumir entradas sintaticamente corretas (não é obrigatório tratar erros de sintaxe).

## Estrutura do projeto

- `mnem2op.c` - implementação do montador
- `op2mnem.c` - implementação do desmontador
- `mnem2op-old.c` - versão antiga/protótipo
- `makefile` - build e testes
- `scripts/test_asm.sh` - testes com saída esperada do montador
- `scripts/test_roundtrip.sh` - testes de round-trip
- `scripts/test_ref.sh` - comparação contra `mvn-cli`
- `scripts/test_monitor.sh` - comparação de execução no `mvnMonitor`
- `arquivos-teste/` - programas de referência do EP

## Build

Compilar tudo:

```bash
make
```

Limpar binários:

```bash
make clean
```

## Uso

Montador:

```bash
./mnem2op arquivo.asm
```

Desmontador:

```bash
./op2mnem arquivo.mvn
```

## Convenções implementadas

### `mnem2op` (montador)

- `@` para endereçamento absoluto
- `K` para atribuição de valores
- `/` para hexadecimal e `=` para decimal
- rótulos definidos à esquerda
- saída em linhas no formato:

```text
<ENDERECO4_HEX> <PALAVRA4_HEX>
```

### `op2mnem` (desmontador)

- gera assembly válido a partir de `.mvn`
- usa rótulos no formato pedido no enunciado:
  - `ROTxx` para variáveis/constantes
  - `JUMPxx` para saltos
  - `SUBxx` para subrotinas
- considera a convenção do EP:
  - instruções em `0x0000..0x02FF`
  - dados em `0x0300..0x03FF`

Observação: os nomes/ordem dos rótulos podem diferir dos exemplos originais, mas o resultado continua válido se o round-trip preserva o `.mvn`.

## Testes

Executar todos os testes:

```bash
make test
```

O alvo `test` roda:

1. `test-asm`: compara a saída do `mnem2op` com os arquivos `.mvn` esperados.
2. `test-roundtrip`: valida `.mvn -> op2mnem -> mnem2op` contra o `.mvn` original.

Diferenças apenas de newline final são tratadas como equivalentes.

Testes opcionais com ferramentas de referência da disciplina:

```bash
make test-ref
make test-monitor
```

- `test-ref` compara montagem/desmontagem com `mvn-cli`.
- `test-monitor` compara a execução de binários no `mvnMonitor`.

## Referências MVN (usadas na disciplina)

- https://github.com/PCS3616/mvn-rs
- https://github.com/PCS-Poli-USP/MVN

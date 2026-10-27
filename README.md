# Mandelbrot MPI

Projeto acadêmico em C++ que utiliza **MPI** para distribuir o cálculo
do fractal de Mandelbrot entre três processos e **X11** para exibir a
imagem.

## Requisitos

Em cada máquina Linux usada no projeto, instale o compilador C++, o Open
MPI, o Make e as bibliotecas de desenvolvimento do X11.

Em Debian ou Ubuntu:

``` bash
sudo apt update
sudo apt install build-essential make openmpi-bin libopenmpi-dev libx11-dev openssh-client openssh-server
```

Confira se o MPI está instalado:

``` bash
mpic++ --version
mpirun --version
```

## Compilar

Clone o repositório e entre na pasta do projeto:

``` bash
git clone https://github.com/DeveloperKairo/mandelbrot-mpi
cd mandelbrot-mpi
```

Compile:

``` bash
make clean
make
```

O executável esperado é `mandelbrot`, na raiz do projeto. Se o
`Makefile` ou o nome do executável diferirem, siga os nomes presentes no
repositório.

## Executar em uma única máquina

Para testar o programa com três processos MPI na mesma máquina:

``` bash
mpirun -np 3 ./mandelbrot
```

Execute em uma sessão gráfica Linux, pois o programa usa X11 para exibir
o fractal. Siga a instrução apresentada no terminal para encerrar o
programa.

## Executar em três máquinas

Para distribuir os três processos entre o computador principal e duas
VMs Linux, configure primeiro o SSH entre o computador principal e as
VMs.

### 1. Descobrir os IPs das VMs

Dentro de cada VM:

``` bash
hostname
hostname -I
```

Use os IPs atuais das VMs; eles podem ser diferentes dos utilizados por
outro integrante.

### 2. Configurar SSH

No computador principal, crie uma chave SSH se ainda não tiver uma:

``` bash
ssh-keygen -t ed25519
```

Copie a chave pública para cada VM, substituindo usuário e IP:

``` bash
ssh-copy-id USUARIO@IP_DA_VM1
ssh-copy-id USUARIO@IP_DA_VM2
```

Teste as conexões:

``` bash
ssh USUARIO@IP_DA_VM1 hostname
ssh USUARIO@IP_DA_VM2 hostname
```

Os comandos devem mostrar o nome de cada VM sem solicitar a senha da
conta remota. Não compartilhe sua chave privada SSH.

### 3. Criar o arquivo `hosts`

Na pasta do projeto, no computador principal, crie o arquivo:

``` bash
nano hosts
```

Adicione três linhas, substituindo os endereços de exemplo pelos
endereços atuais:

``` text
localhost slots=1
IP_DA_VM1 slots=1
IP_DA_VM2 slots=1
```

Salve e confira:

``` bash
cat hosts
```

### 4. Copiar o executável para as VMs

Após compilar no computador principal, copie o executável para cada VM:

``` bash
scp ./mandelbrot USUARIO@IP_DA_VM1:/tmp/mandelbrot
scp ./mandelbrot USUARIO@IP_DA_VM2:/tmp/mandelbrot
```

Garanta a permissão de execução:

``` bash
chmod +x ./mandelbrot
ssh USUARIO@IP_DA_VM1 'chmod +x /tmp/mandelbrot'
ssh USUARIO@IP_DA_VM2 'chmod +x /tmp/mandelbrot'
```

O executável precisa estar disponível no caminho usado em cada nó. Neste
exemplo, ele fica em `/tmp/mandelbrot` nas VMs e em `./mandelbrot` no
computador principal. Para usar o mesmo caminho em todos, copie também o
executável do mestre para `/tmp/mandelbrot` e utilize esse caminho no
comando MPI.

### 5. Testar e executar

Primeiro, teste se o MPI consegue iniciar processos nas três máquinas:

``` bash
mpirun --hostfile hosts -np 3 hostname
```

O resultado deve mostrar os nomes das três máquinas. Se falhar, confira
os IPs, o SSH e a instalação do Open MPI antes de continuar.

Depois, execute o fractal. Para usar o mesmo caminho em todos os nós,
copie o executável do computador principal para `/tmp/mandelbrot`:

``` bash
cp ./mandelbrot /tmp/mandelbrot
```

E então execute:

``` bash
mpirun --hostfile hosts -np 3 /tmp/mandelbrot
```

A janela gráfica deve aparecer no computador principal, em uma sessão
gráfica com X11 disponível.

## Observações

-   Instale o Open MPI pelo gerenciador de pacotes da sua distribuição;
    este repositório não inclui uma instalação própria do MPI.
-   Use versões compatíveis do Open MPI em todas as máquinas.
-   Os IPs no arquivo `hosts` dependem da rede e das configurações das
    VMs de cada pessoa.
-   Redes virtuais em modo NAT podem permitir que o computador
    hospedeiro acesse suas VMs, mas não necessariamente permitem que
    outro computador físico da rede acesse essas mesmas VMs.
-   Nunca envie chaves privadas SSH para o repositório.

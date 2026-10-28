
#!/usr/bin/env bash
set -e

cd "$HOME/mpi-lab"

MPI="$HOME/mpi-lab/openmpi-5.0.7/bin/mpirun"
BIN="$HOME/mpi-lab/mandelbrot"

echo "==> Preparando os diretórios..."
mkdir -p /tmp/mpi-lab
ssh pc01 'mkdir -p /tmp/mpi-lab'
ssh pc02 'mkdir -p /tmp/mpi-lab'

echo "==> Sincronizando o executável..."
cp "$BIN" /tmp/mpi-lab/mandelbrot
scp "$BIN" pc01:/tmp/mpi-lab/mandelbrot
scp "$BIN" pc02:/tmp/mpi-lab/mandelbrot

chmod +x /tmp/mpi-lab/mandelbrot
ssh pc01 'chmod +x /tmp/mpi-lab/mandelbrot'
ssh pc02 'chmod +x /tmp/mpi-lab/mandelbrot'

echo "==> Iniciando Mandelbrot com 3 processos MPI..."
exec "$MPI" \
  --prefix /usr \
  --tag-output \
  -np 3 \
  --hostfile "$HOME/mpi-lab/hosts" \
  /tmp/mpi-lab/mandelbrot

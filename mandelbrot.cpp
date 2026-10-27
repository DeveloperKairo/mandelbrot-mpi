#include "X11Display.h"
#include "Palette.h"
#include "Mandel.h"

#include <mpi.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const unsigned int DefaultWidth = 800;
const unsigned int DefaultHeight = 600;
const int DefaultMaxIter = 20000;
const unsigned int BlockWidth = 8;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 3) {
        if (rank == 0)
            cerr << "Erro: execute com exatamente 3 processos MPI."
                 << endl;

        MPI_Finalize();
        return 1;
    }

    const unsigned int largura = DefaultWidth;
    const unsigned int altura = DefaultHeight;
    const int maxiter = DefaultMaxIter;

    dcomplex zmin(-0.482536, -0.635201);
    dcomplex zmax(-0.565928, -0.579588);

    const unsigned int colunasBase = largura / size;
    const unsigned int resto = largura % size;

    const unsigned int inicio =
        rank * colunasBase +
        min(static_cast<unsigned int>(rank), resto);

    const unsigned int colunasLocais =
        colunasBase +
        (static_cast<unsigned int>(rank) < resto ? 1 : 0);

    const double dx =
        (zmax.real() - zmin.real()) / largura;
    const double dy =
        (zmax.imag() - zmin.imag()) / altura;

    Mandel m;

    X11Display d;
    RGBPalette p(0.4, 0.95, 0.65, 0.45, 0.5, 0.5, 0.5);

    if (rank == 0) {
        char titulo[] = "Mandelbrot - MPI";
        d.open(largura, altura, titulo);
    }

    // Cada processo calcula blocos de colunas dentro de sua faixa.
    for (unsigned int offset = 0;
         offset < colunasLocais;
         offset += BlockWidth) {

        const unsigned int bloco =
            min(BlockWidth, colunasLocais - offset);

        vector<int> resultados(bloco * altura);

        for (unsigned int y = 0; y < altura; ++y) {
            for (unsigned int xLocal = 0;
                 xLocal < bloco;
                 ++xLocal) {

                const unsigned int x = inicio + offset + xLocal;

                dcomplex c(
                    zmin.real() + x * dx,
                    zmin.imag() + y * dy
                );

                resultados[y * bloco + xLocal] =
                    m.calcPoint(c, maxiter);
            }
        }

        if (rank == 0) {
            // Desenha o bloco local assim que termina o cálculo.
            for (unsigned int y = 0; y < altura; ++y) {
                for (unsigned int xLocal = 0;
                     xLocal < bloco;
                     ++xLocal) {

                    const int iter =
                        resultados[y * bloco + xLocal];

                    d.drawPoint(
                        inicio + offset + xLocal,
                        y,
                        p.getColor(
                            static_cast<float>(iter) / maxiter
                        )
                    );
                }
            }

            d.flush();
        } else {
            // Envia o bloco calculado ao processo principal.
            MPI_Send(
                resultados.data(),
                static_cast<int>(resultados.size()),
                MPI_INT,
                0,
                static_cast<int>(offset),
                MPI_COMM_WORLD
            );
        }
    }

    if (rank == 0) {
        // Recebe e desenha os blocos remotos da esquerda para a direita.
        for (int origem = 1; origem < size; ++origem) {
            const unsigned int inicioRemoto =
                origem * colunasBase +
                min(static_cast<unsigned int>(origem), resto);

            const unsigned int colunasRemotas =
                colunasBase +
                (static_cast<unsigned int>(origem) < resto ? 1 : 0);

            for (unsigned int offset = 0;
                 offset < colunasRemotas;
                 offset += BlockWidth) {

                const unsigned int bloco =
                    min(BlockWidth, colunasRemotas - offset);

                vector<int> resultados(bloco * altura);

                MPI_Recv(
                    resultados.data(),
                    static_cast<int>(resultados.size()),
                    MPI_INT,
                    origem,
                    static_cast<int>(offset),
                    MPI_COMM_WORLD,
                    MPI_STATUS_IGNORE
                );

                for (unsigned int y = 0; y < altura; ++y) {
                    for (unsigned int xLocal = 0;
                         xLocal < bloco;
                         ++xLocal) {

                        const int iter =
                            resultados[y * bloco + xLocal];

                        d.drawPoint(
                            inicioRemoto + offset + xLocal,
                            y,
                            p.getColor(
                                static_cast<float>(iter) / maxiter
                            )
                        );
                    }
                }

                d.flush();
            }
        }

        cout << "Fractal concluido. Digite fim para encerrar."
             << endl;

        string command;
        while (cin >> command) {
            if (command == "fim")
                break;
        }

        d.close();
    }

    MPI_Finalize();
    return 0;
}


/*
    Disciplina: Sistemas Operacionais - 2022 - 2
    Faculdade: FACET, UFGD
    Docente: Marcos Paulo Moro
    Discente: Raphael Alexsander Prado dos Santos

    Os materiais utilizados como referência foram adicionados ao relatório.
*/


/* ---------------------------------------------- Bibliotecas ---------------------------------------------- */


#include <algorithm>
#include <process.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <limits>
#include <random>
#include <vector>
#include <math.h>
#include <time.h>


/* ---------------------------------------------- Lista de funcoes ---------------------------------------------- */


int menu();
void submenu(int menu_input);
int tamanhoStringNum(int num);
void destroiMatriz(int r, int c);
bool primality_test(long number);
void criarMatriz(int r, int c, int seed);
void computar_submatriz(void* parametros_funcao);
std::vector<struct submatrix_coord> setarSubmatrizes();
void imprimirExecucao(struct dadosExecucao dados_execucao);
std::string charEspacoVariavel(int size_total, int tam_string);
std::ostream& operator<<(std::ostream os, const struct menuInptVars& inpt_vars);
std::ostream& operator<<(std::ostream os, const struct submatrix_coord& submatriz);
std::ostream& operator<<(std::ostream os, const struct dadosExecucao& dados_execucao);
void benchmark(std::vector<HANDLE>& hThreads, std::vector<struct thread_data>& dados_threads);
void criarThreads(std::vector<HANDLE>& hThreads, std::vector<struct thread_data>& dados_threads);


/* ---------------------------------------------- Estruturas de dados auxiliares ---------------------------------------------- */


// Struct cujos dados sao usados para validar inputs do usuario em relacao ao menu

class menuInptVars {
    private:
        unsigned int cpuThreads;
    public:
        int mx_rows;
        int mx_cols;
        int sbmx_rows;
        int sbmx_cols;
        int nThreads;
        int seed;
        bool mx;
        bool erro_menu;
        bool hThreads_created;
        bool executado;
        bool impressao;
        int* porcentagens;

        menuInptVars() {
            mx_rows = 0;
            mx_cols = 0;
            sbmx_rows = 0;
            sbmx_cols = 0;
            nThreads = 0;
            seed = 0;
            cpuThreads = std::thread::hardware_concurrency(); /* Obtem o numero de threads da CPU */
            mx = false;
            erro_menu = false;
            hThreads_created = false;
            executado = false;
            impressao = false;
            porcentagens = nullptr;
        }

        menuInptVars(int mx_rows, int mx_cols, int sbmx_rows, int sbmx_cols, int nThreads, int seed) {
            this->mx_rows = mx_rows;
            this->mx_cols = mx_cols;
            this->sbmx_rows = sbmx_rows;
            this->sbmx_cols = sbmx_cols;
            this->nThreads = nThreads;
            this->seed= seed;
            this->cpuThreads = std::thread::hardware_concurrency(); /* Obtem o numero de threads da CPU */
            this->mx = false;
            this->erro_menu = false;
            this->hThreads_created = false;
            this->executado = false;
            this->impressao = false;
            this->porcentagens = nullptr;
        }

        int getCpuThreads() {
            return cpuThreads;
        }

        void print() {
            std::cout << "----------------------------Dados Matriz----------------------------\n"
                << "| Numero de Threads: " << nThreads << charEspacoVariavel(46, tamanhoStringNum(nThreads)) << "|\n"
                << "| Tamanho da matriz: " << "M[" << mx_rows << "][" << mx_cols << "]" << charEspacoVariavel(41, tamanhoStringNum(mx_rows) + tamanhoStringNum(mx_cols)) << "|\n"
                << "| Tamanho da submatriz: " << "S[" << sbmx_rows << "][" << sbmx_cols << "]" << charEspacoVariavel(38, tamanhoStringNum(sbmx_rows) + tamanhoStringNum(sbmx_cols)) << "|\n"
                << "| Seed de geracao da matriz: " << seed << charEspacoVariavel(38, tamanhoStringNum(seed)) << "|\n";
        }

        friend std::ostream& operator<<(std::ostream os, const menuInptVars& inpt_vars);
};
std::ostream& operator<<(std::ostream os, const menuInptVars& inpt_vars) {
    os << "----------------------------Dados Matriz----------------------------\n"
        << "| Numero de Threads: " << inpt_vars.nThreads << charEspacoVariavel(46, tamanhoStringNum(inpt_vars.nThreads)) << "|\n"
        << "| Tamanho da matriz: " << "M[" << inpt_vars.mx_rows << "][" << inpt_vars.mx_cols << "]" << charEspacoVariavel(41, tamanhoStringNum(inpt_vars.mx_rows) + tamanhoStringNum(inpt_vars.mx_cols)) << "|\n"
        << "| Tamanho da submatriz: " << "S[" << inpt_vars.sbmx_rows << "][" << inpt_vars.sbmx_cols << "]" << charEspacoVariavel(38, tamanhoStringNum(inpt_vars.sbmx_rows) + tamanhoStringNum(inpt_vars.sbmx_cols)) << "|\n"
        << "| Seed de geracao da matriz: " << inpt_vars.seed << charEspacoVariavel(38, tamanhoStringNum(inpt_vars.seed)) << "|\n";
    return os;
}

// Struct usada para gerar as coordenadas dos pontos inicial e final da submatriz
typedef struct coordinates {
    int x;
    int y;
} coordinates;

// Struct que contem os pontos inicial e final da matriz
struct submatrix_coord {
    struct coordinates top_left;
    struct coordinates bottom_right;
    bool verificada = false;
    bool regular = true;
};

// Struct para manter controle de dados relevantes à thread
struct thread_data {
    short unsigned id;
    int primes = 0;
    bool occupied = false;
    int nThreads = 0;
};

// Armazena os dados da ultima execucao para proposito de impressao e validacao de input do menu
struct dadosExecucao {
    int mx_rows = 0;
    int mx_cols = 0;
    int sbmx_rows = 0;
    int sbmx_cols = 0;
    int nThreads = 0;
    int seed = 0;
    clock_t tempo;
    std::vector<struct submatrix_coord> submatrizes_execucao;
};

// Converte numero em string. Utilizei ao inves do std::to_string porque este da erro com o compilador 
namespace patch
{
    template < typename T > std::string to_string(const T& n)
    {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}


/* ---------------------------------------------- Variaveis Globais ---------------------------------------------- */

menuInptVars inpt_vars;                                 // Variavel que controla varios dados de input do usuario
long** matriz;                                          // Matriz dos dados
HANDLE hMutex1;                                         // Mutex da selecao da submatriz
HANDLE hMutex2;                                         // Mutex da atualizacao do total de primos
long long total_primos = 0;                             // Variavel global do total de primos
std::vector <struct submatrix_coord> submatrizes;       // Vetor com as coordenadas de todas as submatrizes


/* ---------------------------------------------- Main ---------------------------------------------- */


int main() {

    // Variaveis do menu
    int opcao_menu = 0;

    // Varivaveis de Execucao
    std::vector <struct thread_data> dados_threads;
    struct dadosExecucao dados_execucao;
    std::vector<HANDLE> hThreads;
    struct thread_data t_data;
    clock_t start;
    clock_t end;


    // Coloca caracteres com acento
    setlocale(LC_ALL, "Portuguese");

    // Inicializando variaveis referentes as threads
    hMutex1 = CreateMutex(NULL, FALSE, NULL);
    hMutex2 = CreateMutex(NULL, FALSE, NULL);

    // Gerenciador das opcoes de menu
    do {
        switch (opcao_menu = menu()) {
            case 3:
                /* Geracao de matriz */
                criarMatriz(inpt_vars.mx_rows, inpt_vars.mx_cols, inpt_vars.seed);
                submatrizes = setarSubmatrizes();
                inpt_vars.mx = true;
                break;
            case 6:
            
                // Zera a contagem de primos
                total_primos = 0;

                // Criacao as threads 
                criarThreads(hThreads, dados_threads);
            
                // Reabre as submatrizes para avaliacao
                for (int i = 0; i < submatrizes.size(); i++)
                    submatrizes[i].verificada = false;
            
                // Verifica se as threads nao foram criadas
                if (hThreads.size() == 0) {
                    submenu(6);
                    break;
                }
                start = clock();
            
                // Inicializa as threads
                for (int i = 0; i < hThreads.size(); i++)
                    ResumeThread(hThreads[i]);
            
                /* Execucao do algoritmo */
                WaitForMultipleObjects(inpt_vars.nThreads, hThreads.data(), TRUE, INFINITE);
            
                // Suspende as threads
                for (int i = 0; i < hThreads.size(); i++)
                    SuspendThread(hThreads[i]);
                end = clock();

                // Finaliza as threads
                for (int i = 0; i < inpt_vars.nThreads; i++) {
                    CloseHandle(hThreads[i]);
                }
                hThreads.clear();
            
                // Atualiza os dados de impressao
                dados_execucao = { inpt_vars.mx_rows, inpt_vars.mx_cols, inpt_vars.sbmx_rows, inpt_vars.sbmx_cols, inpt_vars.nThreads, inpt_vars.seed, (end - start), submatrizes };
                inpt_vars.executado = true;

                break;
       
            case 7:
                /* Imprimir dados da execucao */
                if (inpt_vars.impressao)
                    imprimirExecucao(dados_execucao);
                break;
            case 9:
                benchmark(hThreads, dados_threads);
                break;
        }
        
        // Verifica se houve alguma alteracao nos dados de execucao, para controlar a impressao correta de resultados
        if (dados_execucao.mx_cols != inpt_vars.mx_cols || dados_execucao.mx_rows != inpt_vars.mx_rows || dados_execucao.sbmx_cols != inpt_vars.sbmx_cols
            || dados_execucao.sbmx_rows != inpt_vars.sbmx_rows || dados_execucao.nThreads != inpt_vars.nThreads || dados_execucao.seed != inpt_vars.seed)
            inpt_vars.executado = false;

    } while (opcao_menu != 8);

    // Desaloca a matriz 
    if (inpt_vars.mx)
        destroiMatriz(inpt_vars.mx_rows, inpt_vars.mx_cols);

    // Finaliza as threads
    if (inpt_vars.nThreads == 0)
        for (int i = 0; i < inpt_vars.nThreads; i++)
            CloseHandle(hThreads[i]);

    CloseHandle(hMutex1);
    CloseHandle(hMutex2);

    return 0;
}


/* ---------------------------------------------- Funcoes de interface ---------------------------------------------- */


int menu() {

    // Variaveis
    int input = -1;
    int secondary_input = 0;

    for (bool laco = true; laco != false; laco = !((input == 8) || !(input == 3) || !(input == 6) || !(input == 7))) {

        inpt_vars.print();
        std::cout << 
            "--------------------------------Menu--------------------------------\n" <<
            "|1) Definir o tamanho da matriz                                    |\n" <<
            "|2) Definir semente para o gerador de numeros aleatorios           |\n" <<
            "|3) Preencher a matriz com numeros aleatorios                      |\n" <<
            "|4) Definir o tamanho das submatrizes                              |\n" <<
            "|5) Definir o numero de Threads                                    |\n" <<
            "|6) Executar                                                       |\n" <<
            "|7) Visualizar o tempo de execucao e quantidade de numeros primos  |\n" <<
            "|8) Encerrar                                                       |\n" <<
            "--------------------------------------------------------------------\n" <<
            " Opcao > "
            << std::endl;
        std::cin >> input;

        // Validação do input do usuario
        if (input < 1 || input > 9) {
            std::cout << "Opcao invalida! Selecione uma opcao correta..." << std::endl;
            std::cin >> input;
        }
        else if (input != 8) {
            submenu(input);
        }
        /*system("cls");*/
    }

    return input;
}

void submenu(int menu_input) {

    int submenu_input = 0;
    char resposta;

    switch (menu_input) {

        /* Linha e coluna da matriz */

        case 1:
            // Obtem os valores de linha e coluna para a matriz

            // Obtem o valor de linhas para a matriz
            std::cout << "Matriz" << std::endl;
            std::cout << "Digite o numero de linhas: " << std::endl;
            std::cin >> inpt_vars.mx_rows;
            // Valida o valor de linhas para a matriz
            while (inpt_vars.mx_rows <= 0) {
                std::cout << "Erro! Digite um valor maior que zero para o numero de linhas!" << std::endl;
                std::cout << "Digite o numero de linhas: " << std::endl;
                std::cin >> inpt_vars.mx_rows;
            }
            // Obtem o valor de colunas para a matriz
            std::cout << "Digite o numero de colunas: " << std::endl;
            std::cin >> inpt_vars.mx_cols;
            // Valida o valor de colunas para a matriz
            while (inpt_vars.mx_cols <= 0) {
                std::cout << "Erro! Digite um valor maior que zero para o numero de colunas!" << std::endl;
                std::cout << "Digite o numero de colunas: " << std::endl;
                std::cin >> inpt_vars.mx_cols;
            }
            break;

            /* Definicao da semente */

        case 2:
            // Obtem o valor para a semente 
            std::cout << "Semente para a geração de valores: " << std::endl;
            std::cin >> inpt_vars.seed;
            // Valida o valor para a semente
            while (inpt_vars.seed <= 0) {
                std::cout << "A semente digitada deve ser maior que zero (0)!" << std::endl;
                std::cout << "Semente para a geração de valores: " << std::endl;
                std::cin >> inpt_vars.seed;
            }
            break;

            /* Validacao para geracao da matriz */

        case 3:
            // Somente executa o codigo de geracao da matriz se as linhas e colunas tiverem sido inseridas para a matriz
            if (inpt_vars.mx_cols == 0 || inpt_vars.mx_rows == 0) {
                std::cout << "Voce se esqueceu de digitar o numero de linhas e/ou colunas da matriz!" << std::endl;
                submenu(1);
                break;
            }
            // Somente executa o codigo de geracao da matriz se as linhas e colunas tiverem sido inseridas para a submatriz
            if (inpt_vars.sbmx_cols == 0 || inpt_vars.sbmx_rows == 0) {
                std::cout << "Voce se esqueceu de digitar o numero de linhas e/ou colunas da submatriz!" << std::endl;
                inpt_vars.erro_menu = true;
                submenu(4);
                break;
            }
            // Verifica se a semente foi digitada
            if (!inpt_vars.seed) {
                std::cout << "Voce se esqueceu de digitar o numero da semente! Pressione qualquer" << std::endl;
                submenu(2);
                break;
            }
            // Verifica se a matriz ja foi criada anteriormente e a apaga se sim
            if (inpt_vars.mx) {
                destroiMatriz(inpt_vars.mx_rows, inpt_vars.mx_cols);
                inpt_vars.mx = true;
            }

            break;

            /* Tamanho das submatrizes */

        case 4:
            // Obtem os valores de linha e coluna para a submatriz

            // Verifica se a matriz ja foi definida. Se nao foi, impede de inserir os valores da submatriz
            if (inpt_vars.mx_cols == 0 || inpt_vars.mx_rows == 0) {
                std::cout << "Erro! Defina primeiro o numero de linhas e colunas da matriz!" << std::endl;
                getchar();
                break;
            }
            // Obtem o valor de linhas para a submatriz
            std::cout << "Submatriz" << std::endl;
            std::cout << "Digite o numero de linhas: ";
            std::cin >> inpt_vars.sbmx_rows;
            // Valida os valores de linha para a submatriz em relacao a numeros positivos
            while (inpt_vars.sbmx_rows <= 0) {
                std::cout << "Erro! Digite um valor maior que zero para o numero de linhas!" << std::endl;
                std::cout << "Digite o numero de linhas: ";
                std::cin >> inpt_vars.sbmx_rows;
            }
            // Valida os valores de linha para a submatriz em relacao ao numero de linhas da matriz
            while (inpt_vars.sbmx_rows > inpt_vars.mx_rows) {
                std::cout << "Erro! O numero de linhas da submatriz deve ser, no maximo, igual ao da matriz!" << std::endl;
                std::cout << "Digite o numero de linhas: ";
                std::cin >> inpt_vars.sbmx_rows;
            }
            // Obtem o valor de colunas para a matriz
            std::cout << "Digite o numero de colunas: ";
            std::cin >> inpt_vars.sbmx_cols;
            // Valida os valores de coluna para a submatriz em relacao a numeros positivos
            while (inpt_vars.sbmx_cols <= 0) {
                std::cout << "Erro! Digite um valor maior que zero para o numero de colunas!" << std::endl;
                std::cout << "Digite o numero de colunas: ";
                std::cin >> inpt_vars.sbmx_cols;
            }
            // Valida os valores de coluna para a submatriz em relacao ao numero de colunas da matriz
            while (inpt_vars.sbmx_rows > inpt_vars.mx_rows) {
                std::cout << "Erro! O numero de colunas da submatriz deve ser, no maximo, igual ao da matriz!" << std::endl;
                std::cout << "Digite o numero de colunas: ";
                std::cin >> inpt_vars.sbmx_rows;
            }
            break;

            /* Numero de Threads */

        case 5:
            do {
                // Obtem o valor do numero de threads
                std::cout << "Numero de threads: ";
                std::cin >> inpt_vars.nThreads;
                // Validacao do valor do numero de threads em relacao ao valor positivo
                while (inpt_vars.nThreads <= 0) {
                    std::cout << "Erro! Digite um valor maior que zero para o numero de threads!" << std::endl;
                    std::cout << "Numero de threads: ";
                    std::cin >> inpt_vars.nThreads;
                }
                // Validacao do numero de threads em relacao ao numero de threads da CPU
                if (inpt_vars.nThreads > inpt_vars.getCpuThreads()) {
                    std::cout << "O valor de threads escolhido e maior que o numero de threads neste processador," <<
                        "podendo causar perda de performance. Deseja continuar mesmo assim? (s/N)"
                        << std::endl;
                    std::cin >> resposta;
                    if (resposta == 83 || resposta == 115)
                        break;
                }

            } while (inpt_vars.nThreads > inpt_vars.getCpuThreads());

            break;

            /* Execucao do algoritmo de calculo de numeros primos */

        case 6:
            if (!inpt_vars.getCpuThreads() || !inpt_vars.mx_cols || !inpt_vars.mx_rows || !inpt_vars.sbmx_cols || !inpt_vars.sbmx_rows || !inpt_vars.seed)
                std::cout << std::endl << "Erro! Os seguintes campos ainda precisam ser preenchidos: \n"
                    << (!inpt_vars.nThreads ? "5) Numero de threads\n" : "") 
                    << (!inpt_vars.mx_cols || !inpt_vars.mx_rows ? "1) Definir o numero de linhas e colunas da matriz\n" : "") 
                    << (!inpt_vars.sbmx_cols || !inpt_vars.sbmx_rows ? "4) Definir o numero de linhas e colunas da submatriz\n" : "") 
                    << (!inpt_vars.seed ? "2) Definir a seed de geracao de numeros aleatorios\n" : "")
                    << std::endl;
            if (!inpt_vars.mx)
                submenu(3);
            if (!inpt_vars.nThreads)
                submenu(5);
            break;
        case 7:
            if (!inpt_vars.executado) {
                do {
                    std::cout << "Atencao! Os dados de execucao a serem impressos nao sao referentes aos dados atuais que voce inseriu. Deseja prosseguir?" << std::endl;
                    std::cout << "> (s/N): ";
                    std::cin >> resposta;
                } while (resposta != 's' && resposta != 'S' && resposta != 'n' && resposta != 'N');

                if (resposta == 's' || resposta == 'S')
                    inpt_vars.impressao = true;
            }
            else
                inpt_vars.impressao = true;
            
            break;
        case 9:
            break;
    }
}


/* ---------------------------------------------- Funcoes de impressao ---------------------------------------------- */


void imprimirExecucao(struct dadosExecucao dados_execucao) {
    std::cout << "Numero de Threads: " << dados_execucao.nThreads << "\n"
              << "Tamanho da matriz: " << "M[" << dados_execucao.mx_rows << "][" << dados_execucao.mx_cols << "]\n" 
              << "Tamanho da submatriz: " << "S[" << dados_execucao.sbmx_rows << "][" << dados_execucao.sbmx_cols << "]\n" 
              << "Seed de geracao da matriz: " << dados_execucao.seed << std::endl
              << "Primos encontrados: " << total_primos << "\n" 
              << "Tempo de execucao: " << ((double)dados_execucao.tempo) / CLOCKS_PER_SEC << "s"
              << std::endl;;
}


std::string charEspacoVariavel(int size_total, int tam_string) {

    std::string temp;

    for (int i = tam_string; i < size_total; i++)
        temp.append(" ");

    return temp;
}

int tamanhoStringNum(int num) {
    std::string temp = patch::to_string(num);
    return temp.size();
}

std::ostream& operator<<(std::ostream os, const struct submatrix_coord& submatriz) {
    os << "{" << submatriz.top_left.x << ","
        << "" << submatriz.top_left.y << "}\n"
        << "{" << submatriz.bottom_right.x << ","
        << "" << submatriz.bottom_right.y << "}\n";
    return os;
}

std::ostream& operator<<(std::ostream os, const struct dadosExecucao& dados_execucao) {
    os << "Numero de Threads: " << dados_execucao.nThreads << "\n" <<
        "Tamanho da matriz: " << "M[" << dados_execucao.mx_rows << "][" << dados_execucao.mx_cols << "]\n" <<
        "Tamanho da submatriz: " << "S[" << dados_execucao.sbmx_rows << "][" << dados_execucao.sbmx_cols << "]\n" <<
        "Seed de geracao da matriz: " << dados_execucao.seed << "\n";
    return os;
}


/* ---------------------------------------------- Funcoes de Matriz ---------------------------------------------- */


void criarMatriz(int r, int c, int seed) {
    // Gera a semente fixa
    //srand(seed);

    // Por enquanto usa um valor aleatorio, precisa ser trocado
    std::random_device rd;
    std::mt19937_64 eng(rd());
    // Cria uma distribuicao de valores que vai de 0 a LONG_MAX
    std::uniform_int_distribution<unsigned long> distr;

    // Aloca a matriz 
    matriz = new long* [r];
    for (int i = 0; i < r; i++)
        matriz[i] = new long[c];

    // Associando os valores da matriz
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            matriz[i][j] = distr(eng) % 100000000;
        }
    }
}

void destroiMatriz(int r, int c) {
    // Desaloca a matriz (caso ela nao tenha sido alocada, nao tem problema)
    for (int i = 0; i < r; i++)
        delete[] matriz[i];
    delete[] matriz;
}

std::vector<struct submatrix_coord> setarSubmatrizes() {

    // Recebe o numero total de elementos que uma unica submatriz contem
    long capacity = inpt_vars.sbmx_cols * inpt_vars.sbmx_rows;
    // Numero de divisoes de linhas da submatriz exatas
    int R_size = inpt_vars.mx_rows / inpt_vars.sbmx_rows;
    // Numero de divisoes de colunas da submatriz exatas
    int C_size = inpt_vars.mx_cols / inpt_vars.sbmx_cols;
    // Numero de blocos não exatos que podem ser obtidos do restante de linhas
    long remainder_R = (inpt_vars.mx_rows - R_size * inpt_vars.sbmx_rows);
    // Numero de blocos não exatos que podem ser obtidos do restante de colunas
    long remainder_C = (inpt_vars.mx_cols - C_size * inpt_vars.sbmx_cols);
    // Numero da linha a partir do qual a matriz ja nao pode ser dividida perfeitamente 
    int limit_R = inpt_vars.mx_rows - inpt_vars.mx_rows % inpt_vars.sbmx_rows - 1;
    // Numero da linha a partir do qual a matriz ja nao pode ser dividida perfeitamente 
    int limit_C = inpt_vars.mx_cols - inpt_vars.mx_cols % inpt_vars.sbmx_cols - 1;

    std::vector<struct submatrix_coord> submatrizes;
    coordinates fim_matriz = {inpt_vars.mx_rows, inpt_vars.mx_cols};

    // Variavel de controle do loop
    int c = 0;

    std::cout << "Capacidade (submatriz): " << capacity << std::endl;
    std::cout << "R_size: " << R_size << ", C_size: " << C_size << std::endl;
    std::cout << "remainder_R: " << remainder_R << ", remainder_C: " << remainder_C << std::endl;
    std::cout << "Threads (CPU): " << inpt_vars.getCpuThreads() << std::endl;

    // Designando endereço dos blocos regulares
    for (int i = 0; i < R_size; i++) {
        for (int j = 0; j < C_size; j++) {
            submatrizes.push_back({
                    { { i * inpt_vars.sbmx_rows  }, { j * inpt_vars.sbmx_cols } },
                    { { i * (inpt_vars.sbmx_rows) + inpt_vars.sbmx_rows - 1 }, { j * inpt_vars.sbmx_cols + inpt_vars.sbmx_cols - 1 } }
                });
            /*std::cout << "A[" << c << "]"
                      << "{" << submatrizes[c].top_left.x << ","
                      << "" << submatrizes[c].top_left.y << "},"
                      << "{" << submatrizes[c].bottom_right.x << ","
                      << "" << submatrizes[c].bottom_right.y << "}\n";*/
            c++;
        }
    }

    // Designando endereço dos blocos irregulares
    int temp_l = limit_C + 1;
    int temp_k = 0;
    int k = 0;
    int l;
    int iregular_sbmxcapacity = 0;

    // Verifica se a matriz tem linhas restantes
    remainder_C == 0 ? 
        (remainder_R == 0 ? 
            (k = fim_matriz.x) : k = limit_R + 1) 
        : k = 0;
    for (; k < fim_matriz.x; k++) {
        remainder_C == 0 ? l = 0 : l = limit_C + 1;
        for (; l < fim_matriz.y; l++) {
            iregular_sbmxcapacity++;
            if (iregular_sbmxcapacity == capacity || (l == (inpt_vars.mx_cols - 1) && k == (inpt_vars.mx_rows - 1))) {
                submatrizes.push_back({
                    { { temp_k  }, { temp_l } },
                    { { k }, { l } }
                    , false, false});
                temp_k = (l + 1) == inpt_vars.mx_cols ? k + 1 : k = k;
                temp_l = l + 1;
                /*std::cout << "S[" << c << "]"
                          << "{" << submatrizes[c].top_left.x << ","
                          << "" << submatrizes[c].top_left.y << "},"
                          << "{" << submatrizes[c].bottom_right.x << ","
                          << "" << submatrizes[c].bottom_right.y << "}"
                          << " - Capacidade: " << iregular_sbmxcapacity << "\n";*/
                iregular_sbmxcapacity = 0;
                c++;
            }
            if (k == limit_R && l == (fim_matriz.y - 1) && limit_R != (inpt_vars.mx_rows - 1)) {
                l = 0;
                break;
            }
        }
    }

    return submatrizes;
}


/* ---------------------------------------------- Funcoes de threads ---------------------------------------------- */


void criarThreads(std::vector<HANDLE>& hThreads, std::vector<struct thread_data>& dados_threads) {

    struct thread_data t_data;
    t_data.nThreads = inpt_vars.nThreads;

    dados_threads.clear();

    for (unsigned short i = 0; i < inpt_vars.nThreads; i++) {
        t_data = {i, 0, false, inpt_vars.nThreads};
        dados_threads.push_back(t_data);
    }

    for(int i = 0; i < inpt_vars.nThreads; i++)
        hThreads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&computar_submatriz, &dados_threads[i], CREATE_SUSPENDED, NULL));

}

void computar_submatriz(void* parametros_funcao) {

    thread_data* thread_param = (thread_data*)parametros_funcao;
    struct submatrix_coord submatriz;
    int submatrizes_verificadas = 0;
    thread_param->primes = 0;
    bool prim_submx = true;

    while (submatrizes_verificadas != submatrizes.size()) {

        // Calcula os primos da primeira submatriz (definida atraves do id)
        if (prim_submx) {
            if (thread_param->id < submatrizes.size()) {
                submatriz = submatrizes[thread_param->id];
                submatrizes[thread_param->id].verificada = true;
                submatrizes_verificadas++;
                prim_submx = false;
                std::cout << (((float)100 / submatrizes.size()) * submatrizes_verificadas) << "%" << std::endl;
            }
            else
                return;
        }
        // A thread seleciona uma submatriz que ainda nao tenha sido verificada
        else {
            submatrizes_verificadas = inpt_vars.nThreads;
            WaitForSingleObject(hMutex1, INFINITE);
            for (int i = inpt_vars.nThreads < submatrizes.size() ? inpt_vars.nThreads : submatrizes.size(); i < submatrizes.size(); i++) {
                if (!submatrizes[i].verificada) {
                    submatrizes[i].verificada = true;
                    submatriz = submatrizes[i];
                    std::cout << (((float)100 / submatrizes.size()) * submatrizes_verificadas) << "%" << std::endl;
                    break;
                }
                else
                    submatrizes_verificadas++;
            }
            ReleaseMutex(hMutex1);
        }
        
        if (submatrizes_verificadas == submatrizes.size())
            return;

        // Percorrer a submatriz e calcular os primos
        for (int i = submatriz.top_left.x; i <= submatriz.bottom_right.x; i++) {
            for (int j = submatriz.top_left.y; j <= submatriz.bottom_right.y; j++)
                if (primality_test(matriz[i][j]))
                    thread_param->primes++;
        }

        // Atualiza o valor na variavel global de contagem de primos
        WaitForSingleObject(hMutex2, INFINITE);
        total_primos += thread_param->primes;
        ReleaseMutex(hMutex2);

        // Atualiza os valores para permitir uma nova execucao
        thread_param->primes = 0;
        thread_param->occupied = false;
        if (submatrizes_verificadas != submatrizes.size())
            submatrizes_verificadas = 0;
    }

    _endthread();
}


/* ---------------------------------------------- Funcoes de primalidade ---------------------------------------------- */


bool primality_test(long number) {

    // Verifica se o numero e multiplo de 2
    if (((number & 1) == 0 && number != 2) || number <= 1)
        return false;
    // Verifica, para 2..raiz(n) se o numero é 
    for (int i = 3; i < (long)sqrt(number) + 1; i += 2)
        if (number % i == 0) {
            return false;
        }
    return true;
}


/* ---------------------------------------------- Funcoes adicionais ---------------------------------------------- */


/* ---------------------------------------------- Benchmark ---------------------------------------------- */

void benchmark(std::vector<HANDLE>& hThreads, std::vector<struct thread_data>& dados_threads) {
    
    // Seta alguns valores, porem os unicos relevantes sao as submatrizes
    inpt_vars = menuInptVars(0, 0, 1000, 1000, 0, 0);
    // Dados base para tetste fixados
    int matrixes[] = { 5000, 10000, 20000, 40000 };
    int threads[] = { 1, 4, 8 };
    // Controle de escrita no arquivo de texto
    std::ofstream arquivo;
    // Calcula o tempo decorrido na execucao
    clock_t start;
    clock_t end;
    int k = 0;

    // Abre o arquivo de armazenamento para escrita
    arquivo.open("benchmark.txt", std::ofstream::trunc);
    
    for (int i = 0; i < 4; i++) {
        
        inpt_vars = menuInptVars(matrixes[i], matrixes[i], 100, 100, 0, 25);
        inpt_vars.mx = true;

        // Aloca a matriz
        criarMatriz(matrixes[i], matrixes[i], 25);
        submatrizes = setarSubmatrizes();

        for (int j = 0; j < 1; j++) {
            for (int m = 0; m < 5; m++) {

                total_primos = 0;

                start = clock();

                // Cria as threads para cada iteração
                inpt_vars.nThreads = threads[j];
                criarThreads(hThreads, dados_threads);
                for (int l = 0; l < hThreads.size(); l++)
                    ResumeThread(hThreads[l]);

                // Executa o código e para aqui
                WaitForMultipleObjects(inpt_vars.nThreads, hThreads.data(), TRUE, INFINITE);

                // Suspende todas as threads
                for (k = 0; k < hThreads.size(); k++)
                    SuspendThread(hThreads[k]);
                end = clock();

                // Remove as threads utilizadas
                for (k = 0; k < inpt_vars.nThreads; k++)
                    CloseHandle(hThreads[k]);
                std::cout << total_primos << " " << ((double)end - start) / CLOCKS_PER_SEC << std::endl;
                arquivo << "M[" << matrixes[j] << "][" << matrixes[j] << "]," << inpt_vars.nThreads << " Thread(s) -> Tempo: " << ((double)end - start) / CLOCKS_PER_SEC << ", primos: " << total_primos << "\n";

                for (int i = 0; i < submatrizes.size(); i++)
                    submatrizes[i].verificada = false;

                hThreads.clear();
            }
        }

        // Desaloca a matriz
        destroiMatriz(matrixes[i], matrixes[i]);
    }

    inpt_vars.mx = false;

    arquivo.close();
}

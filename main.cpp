#include <algorithm>
#include <bitset>
#include <chrono>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

const double MUTATION_RATE = 3;        // 3%
const double CROSSOVER_RATE = 80;      // 80%
const unsigned POPULATION_SIZE = 20;   // 20 indivíduos
const unsigned MAX_GENERATIONS = 1000; // 1000 gerações
const unsigned MAX_RUNS = 50;          // 50 execuções

using namespace std;
using Queens = vector<bitset<3>>; // 3 bits para representar 8 rainhas

// Gera um número aleatório entre a e b
unsigned urand(unsigned, unsigned);

// Preenche o vetor de rainhas com números aleatórios
void populate(Queens &queens);

// Seleciona os pais
tuple<unsigned, unsigned> select_parents(const vector<Queens> &population);

// Calcula o fitness de uma população
// O fitness é o número de pares de rainhas que se atacam
unsigned fitness(const Queens &queens);

// Crossover. Troca os genes dos pais para criar os filhos
tuple<Queens, Queens> crossover(const Queens &parent1, const Queens &parent2);

// Mutação. Inverte um bit aleatório de uma rainha aleatória
void mutate(Queens &queens);

// Rola o dado para selecionar a rainha. Quanto maior o fitness, maior a chance
// de ser selecionada
unsigned roll_fitness(const vector<Queens> &population);

// Retorna a melhor rainha da população
Queens get_best_queen(const vector<Queens> &population);

// Desenha o tabuleiro com as rainhas
void draw_queen(const Queens &queens);

// Imprime o fitness de cada rainha da população
void print_fitness_population(const vector<Queens> &population);

tuple<Queens, unsigned> genetic_algorithm();

// Calcula a média
double mean(const vector<long long> &times);

// Calcula o desvio padrão
double standard_deviation(const vector<long long> &times);

// Ordena a população pelo fitness
void sort_population(vector<Queens> &population);

int main()
{
    vector<Queens> best_queens;    // Melhores rainhas de cada execução
    vector<long long> times;       // Tempos de execução
    vector<long long> generations; // Número de execuções

    best_queens.reserve(MAX_RUNS); // Reserva espaço para as melhores rainhas
    times.reserve(MAX_RUNS);       // Reserva espaço para os tempos
    generations.reserve(MAX_RUNS); // Reserva espaço para o número de execuções

    auto count = 0U;
    while (count < MAX_RUNS)
    {
        std::cout << "===========================================" << std::endl;
        cout << "Run: " << count + 1 << endl;
        Queens best_queen;
        unsigned generation;

        auto start = chrono::high_resolution_clock::now();
        auto result = genetic_algorithm(); // Executa o algoritmo genético
        auto end = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

        tie(best_queen, generation) = result; // Obtém a melhor rainha e a geração

        best_queens.push_back(best_queen); // Adiciona a melhor rainha
        times.push_back(duration.count()); // Adiciona o tempo
        generations.push_back(generation); // Adiciona o número de gerações

        ++count;
    }

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    cout << "===========================================" << endl;
    cout << "Results" << endl;
    cout << "===========================================" << endl;

    for (auto i = 0; i < MAX_RUNS; ++i)
    {
        cout << "Run: " << i + 1 << " -> ";
        cout << "Fitness: " << fitness(best_queens[i]) << ", ";
        cout << "Time: " << times[i] << "ms, ";
        cout << "Generations: " << generations[i] << endl;
    }

    cout << "===========================================" << endl;
    cout << "Mean time: " << mean(times) << "ms" << endl;
    cout << "Standard deviation time: " << standard_deviation(times) << "ms" << endl;
    cout << "Mean generations: " << mean(generations) << endl;
    cout << "Standard deviation generations: " << standard_deviation(generations) << endl;
    cout << "===========================================" << endl;
    cout << "5 best queens" << endl;

    sort_population(best_queens); // Ordena as melhores rainhas

    for (auto i = 0; i < 5; ++i)
    {
        cout << "Fitness: " << fitness(best_queens[i]) << endl;
        cout << "Queens: ";
        for (auto &queen : best_queens[i])
        {
            cout << queen << " ";
        }
        cout << endl;
        draw_queen(best_queens[i]);
    }

    return EXIT_SUCCESS;
}

unsigned urand(unsigned a, unsigned b)
{
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<unsigned> dis(a, b);

    return dis(gen);
}

void populate(Queens &queens)
{
    for (unsigned i = 0; i < 8; ++i)
    {
        queens[i] = urand(0, 7);
    }
}

tuple<unsigned, unsigned> select_parents(const vector<Queens> &population)
{
    unsigned parent1 = roll_fitness(population);
    unsigned parent2 = roll_fitness(population);

    while (parent1 == parent2)
    {
        parent2 = roll_fitness(population);
    }

    return make_tuple(parent1, parent2);
}

unsigned fitness(const Queens &queens)
{
    unsigned fitness = 0;

    for (unsigned i = 0; i < 8; ++i)
    {
        for (unsigned j = i + 1; j < 8; ++j)
        {
            long diff = j - i;
            long diff2 = queens[i].to_ulong() - queens[j].to_ulong();
            if (queens[i] == queens[j] || abs(diff) == abs(diff2))
            {
                ++fitness;
            }
        }
    }

    return fitness;
}

tuple<Queens, Queens> crossover(const Queens &parent1, const Queens &parent2)
{
    Queens child1(8);
    Queens child2(8);

    unsigned crossover_point = urand(0, 7);

    for (unsigned i = 0; i < crossover_point; ++i)
    {
        child1[i] = parent1[i];
        child2[i] = parent2[i];
    }

    for (unsigned i = crossover_point; i < 8; ++i)
    {
        child1[i] = parent2[i];
        child2[i] = parent1[i];
    }

    return make_tuple(child1, child2);
}

void mutate(Queens &queens)
{
    auto n = urand(0, 7); // Seleciona uma rainha aleatória
    auto i = urand(0, 2); // Seleciona um bit aleatório

    queens[n].flip(i);
}

unsigned roll_fitness(const vector<Queens> &population)
{
    unsigned total_fitness = 0;

    for (const auto &queens : population)
    {
        total_fitness += fitness(queens); // Soma o fitness de todas as rainhas
    }

    unsigned roll = urand(0, total_fitness - 1);

    unsigned sum = 0;
    for (unsigned i = 0; i < POPULATION_SIZE; ++i)
    {
        sum += fitness(population[i]);
        if (sum >= roll)
        {
            return i;
        }
    }

    // Nunca deve chegar aqui. Apenas para evitar warnings
    return (unsigned)population.size() - 1;
}

Queens get_best_queen(const vector<Queens> &population)
{
    auto best_queen = min_element(population.begin(), population.end(),
                                  [](const Queens &a, const Queens &b) { return fitness(a) < fitness(b); });

    return *best_queen;
}

void draw_queen(const Queens &queens)
{
    for (unsigned i = 0; i < 8; ++i)
    {
        for (unsigned j = 0; j < 8; ++j)
        {
            if (queens[j].to_ulong() == i)
            {
                cout << "Q ";
            }
            else
            {
                cout << ". ";
            }
        }
        cout << endl;
    }
}

void print_fitness_population(const vector<Queens> &population)
{
    cout << "Fitness: ";
    for (const auto &queens : population)
    {
        cout << fitness(queens) << " ";
    }
    cout << endl;
}

tuple<Queens, unsigned> genetic_algorithm()
{
    // Inicializa a população
    // 20 individuos de 8 rainhas. Cada rainha é representada por 3 bits
    vector<Queens> population(POPULATION_SIZE, Queens(8));

    // Popula a população
    for (auto &queens : population)
    {
        populate(queens);
    }

    // Encontra a melhor rainha
    auto best_queen = get_best_queen(population);

    unsigned generation = 0;
    while (generation < MAX_GENERATIONS && fitness(best_queen) > 0)
    {
        ++generation;

        vector<Queens> new_population;

        // Adiciona a melhor rainha à nova população. Eltismo de 1
        new_population.push_back(best_queen);

        // Popula a nova população
        while (new_population.size() < POPULATION_SIZE)
        {
            unsigned parent1, parent2; // Seleciona os pais
            tie(parent1, parent2) = select_parents(population);

            Queens child1(8), child2(8); // Cria os filhos

            // Crossover
            if (urand(1, 100) <= CROSSOVER_RATE)
            {
                tie(child1, child2) = crossover(population[parent1], population[parent2]);
            }
            else
            {
                child1 = population[parent1];
                child2 = population[parent2];
            }

            // Mutação
            if (urand(1, 100) <= MUTATION_RATE)
            {
                mutate(child1);
            }
            if (urand(1, 100) <= MUTATION_RATE)
            {
                mutate(child2);
            }

            // Adiciona os filhos à nova população
            if (new_population.size() < POPULATION_SIZE)
            {
                new_population.push_back(child1);
            }
            if (new_population.size() < POPULATION_SIZE)
            {
                new_population.push_back(child2);
            }
        }

        // Atualiza a população
        population = new_population;

        // Atualiza a melhor rainha
        best_queen = get_best_queen(population);
    }

    std::cout << "Generation: " << generation << std::endl;
    std::cout << "Best fitness: " << fitness(best_queen) << std::endl;
    draw_queen(best_queen);
    return make_tuple(best_queen, generation);
}

double mean(const vector<long long> &values)
{
    auto sum = 0.0;
    for (auto value : values)
    {
        sum += value;
    }

    return sum / values.size();
}

double standard_deviation(const vector<long long> &values)
{
    auto m = mean(values);
    auto sum = 0.0;
    for (auto value : values)
    {
        sum += pow(value - m, 2);
    }

    return sqrt(sum / values.size());
}

void sort_population(vector<Queens> &population)
{
    sort(population.begin(), population.end(),
         [](const Queens &a, const Queens &b) { return fitness(a) < fitness(b); });
}

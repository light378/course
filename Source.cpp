#include <iostream>
#include <vector>
#include <string>
#include <limits>

// клас для розв’язання головоломки slitherlink
class slitherlink {
private:
    int r, c;  // розміри сітки (рядки і стовпці)
    std::vector<std::vector<int>> cl;  // підказки в клітинках (-1 якщо немає підказки)
    std::vector<std::vector<bool>> hl, vl;  // горизонтальні і вертикальні лінії (частини циклу)

    int bestlen;  // найкраща (найкоротша) довжина циклу, знайдена досі
    std::vector<std::vector<bool>> besthl, bestvl;  // збережені лінії найкращого розв’язку

    bool line_ok(int, int, bool) {
        return true;  // заглушка для можливого розширення перевірок ліній
    }

    // підрахунок поточної кількості ліній у циклі
    int cur_len() {
        int cnt = 0;
        for (auto& row : hl) for (bool b : row) if (b) cnt++;
        for (auto& row : vl) for (bool b : row) if (b) cnt++;
        return cnt;
    }

    // перевірка валідності частини циклу, що додається
    bool valid_part(int rr, int cc, bool h) {
        int cur = cur_len();
        if (cur >= bestlen) return false;  // не продовжуємо, якщо довжина не краща

        // визначення клітинок, що торкаються цієї лінії
        std::vector<std::pair<int, int>> cells = h
            ? std::vector<std::pair<int, int>>{{rr - 1, cc}, { rr, cc }}
        : std::vector<std::pair<int, int>>{ {rr, cc - 1}, {rr, cc} };

        // перевірка, чи не порушуємо підказки у цих клітинках
        for (auto& p : cells) {
            int rr2 = p.first, cc2 = p.second;
            if (rr2 >= 0 && rr2 < r && cc2 >= 0 && cc2 < c && cl[rr2][cc2] != -1) {
                int cnt = lines_around(rr2, cc2);  // лінії навколо клітинки
                int maxp = cnt + poss_edges(rr2, cc2);  // макс можливі лінії з урахуванням потенційних
                if (cnt > cl[rr2][cc2] || maxp < cl[rr2][cc2]) return false;  // порушення підказки
            }
        }
        return node_deg_ok(rr, cc, h);  // перевірка, щоб степінь вузла не був більшим за 2
    }

    // рекурсивний backtracking для пошуку розв’язку
    bool bt(int rr, int cc, bool h) {
        if (rr == r + 1 && h) return bt(0, 0, false);  // після горизонтальних ліній переходимо до вертикальних
        if (rr == r && !h) {
            if (valid()) {  // якщо поточне рішення валідне
                int length = cur_len();
                if (length < bestlen) {  // оновлення найкращого рішення
                    bestlen = length;
                    besthl = hl;
                    bestvl = vl;
                }
            }
            return false;  // повернення, завершення гілки пошуку
        }
        if (h) {
            hl[rr][cc] = true;  // пробуємо додати горизонтальну лінію
            if (valid_part(rr, cc, true))
                bt_next(rr, cc, true);
            hl[rr][cc] = false;  // пробуємо без лінії
            if (valid_part(rr, cc, true))
                bt_next(rr, cc, true);
        }
        else {
            vl[rr][cc] = true;  // пробуємо додати вертикальну лінію
            if (valid_part(rr, cc, false))
                bt_next(rr, cc, false);
            vl[rr][cc] = false;  // пробуємо без лінії
            if (valid_part(rr, cc, false))
                bt_next(rr, cc, false);
        }
        return false;
    }

    // перехід до наступної лінії у backtracking
    void bt_next(int rr, int cc, bool h) {
        if (h) {
            if (cc + 1 < c) bt(rr, cc + 1, true);
            else bt(rr + 1, 0, true);
        }
        else {
            if (cc + 1 < c + 1) bt(rr, cc + 1, false);
            else bt(rr + 1, 0, false);
        }
    }

    // кількість ліній, що оточують клітинку
    int lines_around(int rr, int cc) {
        int cnt = 0;
        if (hl[rr][cc]) cnt++;
        if (hl[rr + 1][cc]) cnt++;
        if (vl[rr][cc]) cnt++;
        if (vl[rr][cc + 1]) cnt++;
        return cnt;
    }

    // кількість потенційних ліній, що можна додати навколо клітинки
    int poss_edges(int rr, int cc) {
        int cnt = 0;
        if (!hl[rr][cc]) cnt++;
        if (!hl[rr + 1][cc]) cnt++;
        if (!vl[rr][cc]) cnt++;
        if (!vl[rr][cc + 1]) cnt++;
        return cnt;
    }

    // ступінь вершини (кількість ліній, що сходяться у вузлі)
    int node_deg(int rr, int cc) {
        int d = 0;
        if (rr > 0 && vl[rr - 1][cc]) d++;
        if (rr < r && vl[rr][cc])   d++;
        if (cc > 0 && hl[rr][cc - 1]) d++;
        if (cc < c && hl[rr][cc])   d++;
        return d;
    }

    // перевірка, що степінь вузла не більша за 2 (максимум дві лінії)
    bool node_deg_ok(int rr, int cc, bool h) {
        std::vector<std::pair<int, int>> nodes = h
            ? std::vector<std::pair<int, int>>{{rr, cc}, { rr, cc + 1 }}
        : std::vector<std::pair<int, int>>{ {rr, cc}, {rr + 1, cc} };
        for (auto& n : nodes) {
            if (node_deg(n.first, n.second) > 2)
                return false;
        }
        return true;
    }

    // перевірка валідності всього поточного рішення
    bool valid() {
        for (int i = 0; i < r; i++)
            for (int j = 0; j < c; j++)
                if (cl[i][j] != -1 && lines_around(i, j) != cl[i][j])
                    return false;
        return single_loop();  // перевірка, що утворений цикл єдиний і зв’язний
    }

    // перевірка що цикл один і зв’язний
    bool single_loop() {
        int nr = r + 1, nc = c + 1;
        std::vector<std::vector<int>> deg(nr, std::vector<int>(nc));
        for (int i = 0; i < nr; i++)
            for (int j = 0; j < nc; j++)
                deg[i][j] = node_deg(i, j);
        for (int i = 0; i < nr; i++)
            for (int j = 0; j < nc; j++)
                if (deg[i][j] != 0 && deg[i][j] != 2)
                    return false;

        std::vector<std::vector<bool>> vis(nr, std::vector<bool>(nc, false));
        int sr = -1, sc = -1;
        for (int i = 0; i < nr; i++) for (int j = 0; j < nc; j++)
            if (deg[i][j] == 2) { sr = i; sc = j; goto L; }
    L:
        if (sr < 0) return false;

        dfs(sr, sc, vis, deg);  // обхід у глибину для перевірки зв’язності циклу
        for (int i = 0; i < nr; i++)
            for (int j = 0; j < nc; j++)
                if (deg[i][j] == 2 && !vis[i][j])
                    return false;

        return true;
    }

    // обхід у глибину для перевірки зв’язності
    void dfs(int rr, int cc, std::vector<std::vector<bool>>& vis,
        const std::vector<std::vector<int>>& deg) {
        vis[rr][cc] = true;
        int nr = deg.size(), nc = deg[0].size();
        if (rr > 0 && vl[rr - 1][cc] && !vis[rr - 1][cc]) dfs(rr - 1, cc, vis, deg);
        if (rr + 1 < nr && vl[rr][cc] && !vis[rr + 1][cc]) dfs(rr + 1, cc, vis, deg);
        if (cc > 0 && hl[rr][cc - 1] && !vis[rr][cc - 1]) dfs(rr, cc - 1, vis, deg);
        if (cc + 1 < nc && hl[rr][cc] && !vis[rr][cc + 1]) dfs(rr, cc + 1, vis, deg);
    }

public:
    // конструктор класу, ініціалізація всіх полів
    slitherlink(const std::vector<std::vector<int>>& cl0)
        : cl(cl0), r(cl0.size()), c(cl0[0].size()),
        hl(r + 1, std::vector<bool>(c, false)),
        vl(r, std::vector<bool>(c + 1, false)),
        bestlen(std::numeric_limits<int>::max()),
        besthl(hl), bestvl(vl)
    {
    }

    // основна функція розв’язання головоломки
    bool solve() {
        bt(0, 0, true);  // запуск backtracking
        if (bestlen < std::numeric_limits<int>::max()) {
            hl = besthl;
            vl = bestvl;
            return true;
        }
        return false;
    }

    // вивід головоломки і розв’язку у консоль
    void print() {
        for (int rr = 0; rr < r * 2 + 1; rr++) {
            std::string line;
            if (rr % 2 == 0) {
                int li = rr / 2;
                for (int cc = 0; cc < c; cc++) {
                    line += "+";
                    line += hl[li][cc] ? "---" : "   ";
                }
                line += "+";
            }
            else {
                int cr = rr / 2;
                for (int cc = 0; cc < c + 1; cc++) {
                    line += vl[cr][cc] ? "|" : " ";
                    if (cc < c) {
                        int clue = cl[cr][cc];
                        line += (clue == -1 ? "   " : " " + std::to_string(clue) + " ");
                    }
                }
            }
            std::cout << line << "\n";
        }
        std::cout << "\nloop length: " << bestlen << "\n";
    }
};

// отримання цілого числа від користувача з перевіркою
int getint(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        if (std::cin.fail() || value < min || value > max) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\nInvalid input! Please enter a number from " << min << " to " << max << ".\n";
        }
        else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
    }
}

// вивід головного меню
void menu() {
    std::cout << "\n";
    std::cout << "+----------------------------+\n";
    std::cout << "|   Welcome to Slitherlink!  |\n";
    std::cout << "+----------------------------+\n";
    std::cout << "| 1) Play                    |\n";
    std::cout << "| 2) Exit                    |\n";
    std::cout << "+----------------------------+\n";
}

// меню вибору складності
void diff() {
    std::cout << "\n";
    std::cout << "+-----------------------------+\n";
    std::cout << "|      Choose difficulty      |\n";
    std::cout << "+-----------------------------+\n";
    std::cout << "| 0) Beginner                 |\n";
    std::cout << "| 1) Easy                     |\n";
    std::cout << "| 2) Medium                   |\n";
    std::cout << "| 3) Hard                     |\n";
    std::cout << "| 4) Very Hard                |\n";
    std::cout << "| 5) Expert                   |\n";
    std::cout << "+-----------------------------+\n";
}

// меню для продовження або виходу
int cont_menu() {
    std::cout << "\n";
    std::cout << "+-----------------------------+\n";
    std::cout << "|        What next?           |\n";
    std::cout << "+-----------------------------+\n";
    std::cout << "| 1) Continue                 |\n";
    std::cout << "| 2) Exit                     |\n";
    std::cout << "+-----------------------------+\n";
    return getint("Choose option: ", 1, 2);
}

// основна функція програми
int main() {
    // заздалегідь визначені головоломки різних рівнів складності
    std::vector<std::vector<std::vector<int>>> puzzles = {
        {
            {2,-1,-1}, //gпочатківець
            {-1,-1,3},
            {2,-1,2}
        },
        {
           {-1, 2, 0, 2, -1, -1, 2, 2, 3, -1},  //Easy
           {-1, -1, 2, -1, 1, -1, -1, 2, -1, 3},
           {-1, 3, -1, 3, -1, -1, -1, 1, -1, 2},
           {2, 2, -1, -1, -1, -1, -1, 3, -1, -1},
           {1, -1, -1, -1, -1, -1, 2, -1, -1, -1},
           {-1, -1, -1, 2, -1, -1, -1, -1, -1, 3},
           {-1, -1, 3, -1, -1, -1, -1, -1, 2, 2},
           {2, -1, 0, -1, -1, -1, 1, -1, 3, -1},
           {2, -1, 2, -1, -1, 2, -1, 3, -1, -1},
           {-1, 1, 0, 1, -1, -1, 1, 1, 2, -1}
        },
        {
            {3, 1, -1, 2, -1, 1, -1, -1, 3, -1}, //Medium
            {-1, -1, -1, 2, 3, -1, -1, 2, -1, 3},
            {-1, -1, -1, 2, -1, -1, -1, -1, -1, -1},
            {2, -1, 2, 3, -1, 2, 3, 3, -1, 1},
            {-1, -1, -1, -1, 3, -1, 2, -1, -1, -1},
            {-1, -1, 2, -1, 1, 1, 1, 2, -1, -1},
            {2, 1, 2, -1, -1, -1, -1, -1, -1, 3},
            {2, -1, -1, 3, 1, -1, -1, 1, 1, 3},
            {2, -1, -1, 3, 1, 3, -1, -1, -1, -1},
            {-1, -1, -1, 2, -1, -1, 1, -1, 1, -1}
        },
        {
            {-1, 2, -1, 3, -1, 3, 3, -1, 3, -1, 2, -1, 3}, //Hard
            {-1, 1, 2, -1, 2, -1, 0, -1, -1, 3, -1, 3, -1},
            {3, -1, 3, -1, -1, -1, 3, -1, 1, 2, 1, 2, -1},
            {2, -1, 2, -1, 3, 2, -1, -1, -1, 3, -1, 2, -1},
            {-1, -1, 3, 2, -1, 2, -1, 2, -1, 1, 2, -1, -1},
            {-1, 1, -1, 0, -1, -1, -1, 1, 3, -1, 3, -1, 2},
            {-1, 2, 3, 2, 3, -1, 3, -1, -1, -1, 3, -1, 2},
            {-1, 2, -1, 2, -1, -1, 2, -1, 2, -1, 2, 2, -1},
            {3, -1, 3, -1, 2, -1, 2, 3, -1, 3, -1, 3, -1}
        },
        {
            {-1, -1, 3, -1, 1, -1, 3, -1, 3, 3, -1, -1, -1, -1},  //Very Hard
            {1, -1, 1, -1, -1, -1, -1, 1, -1, 1, -1, -1, -1, 3},
            {-1, 3, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, 1, -1},
            {-1, 2, -1, -1, -1, 0, 1, -1, 2, 2, 2, -1, -1, 2},
            {2, 1, -1, 2, 1, 1, -1, -1, 2, -1, -1, -1, -1, 1},
            {2, -1, -1, -1, -1, -1, -1, -1, 2, 2, -1, 2, 1, -1},
            {2, 1, 1, 1, 2, 2, -1, -1, 2, -1, -1, -1, 1, -1},
            {-1, -1, -1, 0, -1, 2, -1, -1, 3, -1, 1, -1, 2, -1},
            {-1, 2, 1, -1, -1, -1, 1, 2, 1, -1, -1, 2, 2, 3},
            {-1, -1, 2, 0, 2, -1, 1, -1, -1, 3, -1, -1, -1, -1},
            {-1, 1, -1, -1, 2, 3, -1, -1, -1, 3, -1, 3, 2, 2},
            {-1, -1, -1, 3, -1, 3, -1, -1, -1, 2, -1, -1, 3, 2},
            {-1, -1, 2, 2, 1, -1, -1, 2, -1, -1, -1, 1, 2, -1},
            {2, 2, -1, -1, -1, 3, -1, -1, -1, -1, -1, -1, 2, 3}
        },
        {
            {-1, 2, 2, 3, -1, 1, 3, -1, -1, -1, -1, 1, 2, 3},  //Expert
            {1, -1, -1, -1, -1, -1, 2, -1, 3, 1, -1, -1, -1, 1},
            {1, -1, 2, -1, -1, 3, -1, -1, -1, -1, 2, 3, 0, -1},
            {-1, 2, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2},
            {2, 1, -1, -1, -1, -1, 1, 2, -1, -1, -1, 2, -1, 1},
            {-1, 2, -1, -1, -1, -1, 3, 3, -1, 2, -1, -1, -1, -1},
            {2, -1, 2, -1, 1, -1, -1, -1, -1, 0, 1, -1, 3, 2},
            {0, 2, -1, 3, 1, -1, -1, -1, -1, 2, -1, 2, -1, 2},
            {-1, -1, -1, -1, 2, -1, 1, 2, -1, -1, -1, -1, 3, -1},
            {2, -1, 3, -1, -1, -1, 3, 1, -1, -1, -1, -1, 2, 3},
            {2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 3, 1, -1},
            {-1, 2, 2, 2, -1, -1, -1, -1, 1, -1, -1, 2, -1, 2},
            {2, -1, -1, -1, 0, 2, -1, 0, -1, -1, -1, -1, -1, 1},
            {1, 3, 1, -1, -1, -1, -1, 2, 1, -1, 2, 2, 1, -1}
        }
    };

    while (true) {
        menu();
        int choice = getint("Choose option: ", 1, 2);
        if (choice == 2) {
            std::cout << "\nGoodbye!\n";
            break;
        }
        else if (choice == 1) {
            while (true) {
                diff();
                int level = getint("Your choice: ", 0, 5);
                std::cout << "\nSolving...\n";
                slitherlink solver(puzzles[level]);
                if (solver.solve()) {
                    std::cout << "\nShortest solution found:\n";
                    solver.print();
                }
                else {
                    std::cout << "No solution.\n";
                }
                std::cout << "--------------------------------\n";
                int next = cont_menu();
                if (next == 2) {
                    std::cout << "Goodbye!\n";
                    return 0;
                }
            }
        }
    }
    return 0;
}

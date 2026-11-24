#include <stdio.h>
#include <string.h>

enum { PC_1, SWITCH_1, PC_2, SWITCH_2, SWITCH_3, SERVER_3, DEVICES };

static const char *const names[DEVICES] = {
    "1호관 실습 PC", "1호관 스위치", "2호관 실습 PC",
    "2호관 스위치", "3호관 스위치", "3호관 제출 서버"
};

typedef struct {
    int link[DEVICES][DEVICES];
} Graph;

typedef struct {
    int parent[DEVICES]; /* -1은 출발지에서 도달하지 못한 장비 */
} Tree;

static void set_link(Graph *graph, int a, int b, int connected) {
    graph->link[a][b] = graph->link[b][a] = connected;
}

/* BFS는 출발지에서 가까운 장비부터 방문한다. 처음 발견한 장비의 부모를 기록한다. */
static Tree shortest_tree(const Graph *graph, int start) {
    Tree tree;
    int queue[DEVICES];
    int head = 0, tail = 0;

    for (int id = 0; id < DEVICES; id++) tree.parent[id] = -1;
    tree.parent[start] = start; /* 트리의 뿌리를 표시 */
    queue[tail++] = start;

    while (head < tail) {
        int current = queue[head++];
        for (int next = 0; next < DEVICES; next++) {
            if (graph->link[current][next] && tree.parent[next] == -1) {
                tree.parent[next] = current;
                queue[tail++] = next;
            }
        }
    }
    return tree;
}

/* 부모를 따라 뿌리까지 이동한 뒤 재귀에서 돌아오며 출발지부터 출력한다. */
static int print_route(const Tree *tree, int device) {
    int parent = tree->parent[device];
    if (parent == device) {
        printf("%s", names[device]);
        return 0;
    }
    int hops = print_route(tree, parent) + 1;
    printf(" -> %s", names[device]);
    return hops;
}

static void show_tree(const Tree *tree, int start) {
    printf("최단 경로 트리 | 출발: %s\n", names[start]);
    for (int id = 0; id < DEVICES; id++) {
        int parent = tree->parent[id];
        if (parent >= 0 && parent != id) printf("  %s -> %s\n", names[parent], names[id]);
    }
}

/* 메뉴는 한 자리 숫자만 받는다. 잘못된 입력은 한 줄을 비우고 다시 받는다. */
static int menu(const char *prompt, int first, int last) {
    char line[64];
    for (;;) {
        fputs(prompt, stdout);
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) return 0;

        if (!strchr(line, '\n') && !feof(stdin)) {
            int remaining;
            do {
                remaining = getchar();
            } while (remaining != '\n' && remaining != EOF);
        } else {
            const char *choice = line + strspn(line, " \t");
            if (*choice >= '0' + first && *choice <= '0' + last &&
                choice[1 + strspn(choice + 1, " \t\r\n")] == '\0') {
                return *choice - '0';
            }
        }
        printf("%d~%d 중 하나를 입력하세요.\n", first, last);
    }
}

static Graph make_graph(int state) {
    Graph graph = {0};
    set_link(&graph, PC_1, SWITCH_1, 1);
    set_link(&graph, PC_2, SWITCH_2, 1);
    set_link(&graph, SWITCH_3, SERVER_3, 1);
    set_link(&graph, SWITCH_1, SWITCH_2, 1);
    set_link(&graph, SWITCH_1, SWITCH_3, state == 1);
    set_link(&graph, SWITCH_2, SWITCH_3, state != 3);
    return graph;
}

int main(void) {
    puts("선린인터넷고 가상 제출망 | SDG 9");
    for (;;) {
        int building = menu("출발 호관 (1: 1호관, 2: 2호관, 0: 종료): ", 0, 2);
        if (building == 0) return 0;

        int state = menu("회선 (1: 정상, 2: 1–3호관 단절, 3: 3호관 회선 모두 단절, 0: 종료): ", 0, 3);
        if (state == 0) return 0;

        int start = building == 1 ? PC_1 : PC_2;
        Graph graph = make_graph(state);
        Tree tree = shortest_tree(&graph, start);

        printf("\n제출 경로: ");
        if (tree.parent[SERVER_3] == -1) {
            puts("통신 불가");
        } else {
            int hops = print_route(&tree, SERVER_3);
            printf(" (%d홉)\n", hops);
        }

        int action = menu("다음 (1: 트리 보기, 2: 다시 선택, 0: 종료): ", 0, 2);
        if (action == 1) {
            show_tree(&tree, start);
            action = menu("다음 (1: 다시 선택, 0: 종료): ", 0, 1);
        }
        if (action == 0) return 0;
        putchar('\n');
    }
}

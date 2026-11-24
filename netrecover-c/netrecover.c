#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DEVICES 20
#define MAX_COST 1000
#define NONE (-1)
#define INF INT_MAX

typedef struct {
    int count;
    const char *name[MAX_DEVICES];
    int cost[MAX_DEVICES][MAX_DEVICES];
    int device_down[MAX_DEVICES];
    int line_down[MAX_DEVICES][MAX_DEVICES];
} Graph;

typedef struct {
    int root;
    int distance[MAX_DEVICES];
    int parent[MAX_DEVICES];
    int first_child[MAX_DEVICES];
    int next_sibling[MAX_DEVICES];
    int order[MAX_DEVICES];
    int reached;
} Tree;

static void connect(Graph *graph, int a, int b, int cost) {
    graph->cost[a][b] = graph->cost[b][a] = cost;
}

static Graph scenario(int choice) {
    if (choice == 2) {
        Graph graph = {.count = 5, .name = {"재난 상황실", "주 중계기", "예비 중계기", "소방서", "병원"}};
        connect(&graph, 0, 1, 2);
        connect(&graph, 1, 4, 2);
        connect(&graph, 0, 2, 5);
        connect(&graph, 2, 3, 2);
        connect(&graph, 3, 4, 3);
        return graph;
    }
    Graph graph = {.count = 6, .name = {"교실 PC", "교실 스위치", "1층 라우터", "2층 라우터", "코어 스위치", "학교 서버"}};
    connect(&graph, 0, 1, 1);
    connect(&graph, 1, 2, 2);
    connect(&graph, 2, 4, 2);
    connect(&graph, 1, 3, 4);
    connect(&graph, 3, 4, 4);
    connect(&graph, 4, 5, 1);
    return graph;
}

/* 정점 하나마다 부모를 하나씩 선택해서 출발지를 뿌리로 하는 트리를 만든다. */
static Tree shortest_tree(const Graph *graph, int source) {
    Tree tree = {.root = source};
    int finalized[MAX_DEVICES] = {0};
    for (int id = 0; id < graph->count; id++) {
        tree.distance[id] = INF;
        tree.parent[id] = tree.first_child[id] = tree.next_sibling[id] = NONE;
    }
    if (graph->device_down[source]) return tree;
    tree.distance[source] = 0;
    for (int step = 0; step < graph->count; step++) {
        int closest = NONE;
        for (int id = 0; id < graph->count; id++) {
            if (!finalized[id] && tree.distance[id] != INF &&
                (closest == NONE || tree.distance[id] < tree.distance[closest])) closest = id;
        }
        if (closest == NONE) break;
        finalized[closest] = 1;
        tree.order[tree.reached++] = closest;
        for (int next = 0; next < graph->count; next++) {
            int cost = graph->cost[closest][next];
            if (cost <= 0 || cost > MAX_COST || graph->line_down[closest][next] ||
                graph->device_down[next] || finalized[next]) continue;
            int candidate = tree.distance[closest] + cost;
            if (candidate < tree.distance[next]) {
                tree.distance[next] = candidate;
                tree.parent[next] = closest;
            }
        }
    }
    /* 역순으로 연결하면 각 부모의 자식이 장비 번호 순으로 표시된다. */
    for (int id = graph->count - 1; id >= 0; id--) {
        int parent = tree.parent[id];
        if (parent == NONE) continue;
        tree.next_sibling[id] = tree.first_child[parent];
        tree.first_child[parent] = id;
    }
    return tree;
}

static void print_branch(const Graph *graph, const Tree *tree, int node, int depth) {
    printf("%*s- %s [비용 %d]\n", depth * 2, "", graph->name[node], tree->distance[node]);
    for (int child = tree->first_child[node]; child != NONE; child = tree->next_sibling[child])
        print_branch(graph, tree, child, depth + 1);
}

/* 목적지에서 부모를 따라 올라간 뒤 역순으로 출력한다. */
static void print_route(const Graph *graph, const Tree *tree, int target) {
    if (tree->distance[target] == INF) {
        puts("통신 불가");
        return;
    }
    int path[MAX_DEVICES];
    int length = 0;
    for (int id = target; id != NONE; id = tree->parent[id]) path[length++] = id;
    for (int i = length - 1; i >= 0; i--)
        printf("%s%s", graph->name[path[i]], i > 0 ? " -> " : "");
    printf(" | 비용 %d | %d홉\n", tree->distance[target], length - 1);
}

static void report(const Graph *graph, int source, int target) {
    Graph normal = *graph;
    memset(normal.device_down, 0, sizeof normal.device_down);
    memset(normal.line_down, 0, sizeof normal.line_down);
    Tree before = shortest_tree(&normal, source);
    Tree after = shortest_tree(graph, source);
    printf("\n[가상 시뮬레이션] %s -> %s\n", graph->name[source], graph->name[target]);
    printf("정상 경로: ");
    print_route(graph, &before, target);
    printf("현재 경로: ");
    print_route(graph, &after, target);
    if (before.distance[target] > 0 && before.distance[target] != INF && after.distance[target] != INF)
        printf("경로 비용 변화율: %+.1f%%\n", 100.0 * (after.distance[target] - before.distance[target]) / before.distance[target]);
    printf("도달 가능 장비: %d/%d -> %d/%d (%.1f%%)\n", before.reached, graph->count,
           after.reached, graph->count, 100.0 * after.reached / graph->count);
    puts("현재 최단 경로 트리:");
    if (after.reached) {
        print_branch(graph, &after, after.root, 0);
        printf("확정 순서:");
        for (int i = 0; i < after.reached; i++) printf(" %d", after.order[i] + 1);
        putchar('\n');
    } else puts("출발 장비 장애로 트리를 생성할 수 없습니다.");
}

static int read_int(const char *prompt, int minimum, int maximum, int *value) {
    char buffer[80];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buffer, sizeof buffer, stdin)) return 0;
        if (!strchr(buffer, '\n') && !feof(stdin)) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
        } else {
            char *end;
            errno = 0;
            long parsed = strtol(buffer, &end, 10);
            if (buffer != end && errno == 0 && parsed >= minimum && parsed <= maximum &&
                end[strspn(end, " \t\r\n")] == '\0') {
                *value = (int)parsed;
                return 1;
            }
        }
        printf("%d~%d 사이 정수를 입력하세요.\n", minimum, maximum);
    }
}

static void show_graph(const Graph *graph) {
    puts("\n장비");
    for (int id = 0; id < graph->count; id++)
        printf("%d. %s%s\n", id + 1, graph->name[id], graph->device_down[id] ? " [장애]" : "");
    puts("회선");
    for (int a = 0; a < graph->count; a++)
        for (int b = a + 1; b < graph->count; b++)
            if (graph->cost[a][b])
                printf("%d-%d 비용 %d%s\n", a + 1, b + 1, graph->cost[a][b],
                       graph->line_down[a][b] || graph->device_down[a] || graph->device_down[b] ? " [사용 불가]" : "");
}

static void default_fault(Graph *graph, int choice) {
    if (choice == 2) graph->device_down[1] = 1;
    else {
        graph->line_down[2][4] = graph->line_down[4][2] = 1;
        if (choice == 3) graph->device_down[3] = 1;
    }
}

int main(void) {
    int choice, source = 0, target;
    puts("NetRecover C | 네트워크 장애 복구");
    puts("1 학교 네트워크  2 도시 재난 통신망  3 완전 단절");
    if (!read_int("시나리오: ", 1, 3, &choice)) return 0;
    Graph graph = scenario(choice);
    target = graph.count - 1;
    show_graph(&graph);
    report(&graph, source, target);
    for (;;) {
        int command, a, b;
        puts("\n1 경로 선택  2 장비 장애/복구  3 회선 장애/복구");
        puts("4 예제 장애  5 모두 복구  6 장비/회선 보기  0 종료");
        if (!read_int("명령: ", 0, 6, &command) || command == 0) break;
        switch (command) {
            case 1:
                if (!read_int("출발 번호: ", 1, graph.count, &a) ||
                    !read_int("목적 번호: ", 1, graph.count, &b)) return 0;
                source = a - 1;
                target = b - 1;
                break;
            case 2:
                if (!read_int("장비 번호: ", 1, graph.count, &a)) return 0;
                graph.device_down[a - 1] = !graph.device_down[a - 1];
                break;
            case 3:
                if (!read_int("회선 첫 장비: ", 1, graph.count, &a) ||
                    !read_int("회선 둘째 장비: ", 1, graph.count, &b)) return 0;
                a--;
                b--;
                if (!graph.cost[a][b]) {
                    puts("해당 회선이 없습니다.");
                    continue;
                }
                graph.line_down[a][b] = graph.line_down[b][a] = !graph.line_down[a][b];
                break;
            case 4:
                default_fault(&graph, choice);
                break;
            case 5:
                graph = scenario(choice);
                break;
            case 6:
                show_graph(&graph);
                continue;
        }
        report(&graph, source, target);
    }
    return 0;
}

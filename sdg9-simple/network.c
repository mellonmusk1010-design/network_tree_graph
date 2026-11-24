#include <stdio.h>

enum { OFFICE, MAIN_RELAY, BACKUP_RELAY, BACKUP_SWITCH, SERVER, DEVICES };

static const char *names[DEVICES] = {
    "공장 사무실", "주 중계기", "예비 중계기", "예비 스위치", "업무 서버"
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

static void show_result(const Graph *graph, const char *title) {
    Tree tree = shortest_tree(graph, OFFICE);

    printf("\n[%s]\n최단 경로 트리 (부모 -> 자식)\n", title);
    for (int id = 0; id < DEVICES; id++) {
        int parent = tree.parent[id];
        if (parent >= 0 && parent != id) printf("  %s -> %s\n", names[parent], names[id]);
    }

    printf("업무 서버까지: ");
    if (tree.parent[SERVER] == -1) {
        puts("통신 불가");
        return;
    }
    int hops = print_route(&tree, SERVER);
    printf(" (%d홉)\n", hops);
}

int main(void) {
    Graph graph = {0};
    set_link(&graph, OFFICE, MAIN_RELAY, 1);
    set_link(&graph, MAIN_RELAY, SERVER, 1);
    set_link(&graph, OFFICE, BACKUP_RELAY, 1);
    set_link(&graph, BACKUP_RELAY, BACKUP_SWITCH, 1);
    set_link(&graph, BACKUP_SWITCH, SERVER, 1);

    puts("SDG 9 | 공장 통신망 복구 시뮬레이션");
    show_result(&graph, "정상");
    set_link(&graph, MAIN_RELAY, SERVER, 0);
    show_result(&graph, "주 회선 장애");
    set_link(&graph, BACKUP_SWITCH, SERVER, 0);
    show_result(&graph, "예비 회선도 장애");
    return 0;
}

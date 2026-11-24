#include <stdio.h>

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

static void show_route(const Graph *graph, int start) {
    Tree tree = shortest_tree(graph, start);

    printf("최단 경로 트리 | 뿌리: %s\n", names[start]);
    for (int id = 0; id < DEVICES; id++) {
        int parent = tree.parent[id];
        if (parent >= 0 && parent != id) printf("  %s -> %s\n", names[parent], names[id]);
    }

    printf("%s에서 제출 서버까지: ", names[start]);
    if (tree.parent[SERVER_3] == -1) {
        puts("통신 불가");
        return;
    }
    int hops = print_route(&tree, SERVER_3);
    printf(" (%d홉)\n", hops);
}

static void show_result(const Graph *graph, const char *title) {
    printf("\n[%s]\n", title);
    show_route(graph, PC_1);
    show_route(graph, PC_2);
}

int main(void) {
    Graph graph = {0};
    set_link(&graph, PC_1, SWITCH_1, 1);
    set_link(&graph, PC_2, SWITCH_2, 1);
    set_link(&graph, SWITCH_3, SERVER_3, 1);
    set_link(&graph, SWITCH_1, SWITCH_3, 1);
    set_link(&graph, SWITCH_2, SWITCH_3, 1);
    set_link(&graph, SWITCH_1, SWITCH_2, 1); /* 건물 사이 예비 회선 */

    puts("SDG 9 | 선린인터넷고 1·2·3호관 가상 통신망");
    show_result(&graph, "정상");
    set_link(&graph, SWITCH_1, SWITCH_3, 0);
    show_result(&graph, "1호관–3호관 회선 장애");
    set_link(&graph, SWITCH_2, SWITCH_3, 0);
    show_result(&graph, "3호관으로 가는 두 회선 모두 장애");
    return 0;
}

/*====================
 * 图的十字链表存储表示
 *
 * 包含算法: 7.3
 ====================*/

#include "OLGraph.h"

// 录入数据的源文件；fp为null时，说明需要从控制台录入
static FILE* fp = NULL;

/*
 * IncInfo指示该图的边上是否存在附加信息。
 * 如果其值不为0，则表示无附加信息，否则，表示存在附加信息。
 */
Boolean IncInfo = FALSE;

// 访问标志数组，记录访问过的顶点
static Boolean visited[MAX_VERTEX_NUM];

// 函数变量
static Status (* VisitFunc)(VertexType e);


/*
 * 创建
 *
 *【备注】
 *
 * 教材中默认从控制台读取数据。
 * 这里为了方便测试，避免每次运行都手动输入数据，
 * 因而允许选择从预设的文件path中读取测试数据。
 *
 * 如果需要从控制台读取数据，则path为NULL，或path[kind]为""。
 * 如果需要从文件中读取数据，则需要在path中填写文件名信息。
 */
Status CreateGraph(OLGraph* G, char* path[]) {
    int readFromConsole;    // 是否从控制台读取数据
    int kind;
    Status flag;
    
    // ★★★此处固定为有向图
    kind = 0;
    
    // 如果没有文件路径信息，则从控制台读取输入
    readFromConsole = (path == NULL) || strcmp(path[kind], "") == 0;
    
    // 需要从文件读取
    if(readFromConsole) {
        // 注：C++文件中，整型转枚举需要强制转换
        (*G).kind = GraphKind(kind);   // 记录图/网的类型
    } else {
        // 打开文件，准备读取测试数据
        fp = fopen(path[kind], "r");
        if(fp == NULL) {
            return ERROR;
        }
        
        // 录入图的类型
        ReadData(fp, "%d", &((*G).kind));
    }
    
    // 随机创建有向图/网或无向图/网的一种
    switch((*G).kind) {
        case DG:
            flag = CreateDG(G);
            break;
        default:
            flag = ERROR;
            break;
    }
    
    if(fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    
    return flag;
}

/*
 * ████████ 算法7.3 ████████
 *
 * 构造有向图
 *
 * 注：
 * 教材中使用了头插法来插入弧，这种做法的唯一优点是插入方便，
 * 但是在涉及到删除或查找时，效率较低；
 * 而且，"头插法"依赖输入的顺序，如果输入的边/弧是乱序的，
 * 则最终构造出的图/网中的边/弧也是无序的。
 *
 * 为了克服以上缺点，这里改用"升序"插入法，保证插入的弧是"升序"的。
 * 但同时，这样做会使"插入"算法变得较为复杂。
 */
static Status CreateDG(OLGraph* G) {
    int i, k;
    int vexnum, arcnum;
    VertexType v1, v2;
    InfoType* info = NULL;
    
    (*G).vexnum = (*G).arcnum = 0;
    
    if(fp == NULL) {
        printf("请输入有向图的顶点数：");
        scanf("%d", &vexnum);
        printf("请输入有向图的弧数：");
        scanf("%d", &arcnum);
        printf("该有向图的弧上是否包含其他附加信息(0-不包含│1-包含)：");
        scanf("%d", &IncInfo);
        
        // 录入顶点集
        printf("请录入 %d 个顶点，不同顶点之间用空格隔开：", vexnum);
        for(i = 0; i < vexnum; i++) {
            scanf("%c", &((*G).xlist[i].data));
            (*G).xlist[i].firstin = NULL;
            (*G).xlist[i].firstout = NULL;
            (*G).vexnum++;
        }
    } else {
        ReadData(fp, "%d", &vexnum);    // 录入顶点数
        ReadData(fp, "%d", &arcnum);    // 录入弧数
        ReadData(fp, "%d", &IncInfo);   // 判断弧上是否包含附加信息
        
        // 录入顶点集
        for(i = 0; i < vexnum; i++) {
            // 跳过空白，寻找下一个"可读"符号
            skipBlank(fp);
            ReadData(fp, "%c", &((*G).xlist[i].data));
            (*G).xlist[i].firstin = NULL;
            (*G).xlist[i].firstout = NULL;
            (*G).vexnum++;
        }
    }
    
    // 仅在控制台录入信息时输出此提示
    if(fp == NULL && arcnum != 0) {
        printf("请为有向图依次录入 %d 条弧的信息，顶点之间用空格隔开：\n", arcnum);
    }
    
    // 录入弧的信息
    for(k = 0; k < arcnum; k++) {
        if(fp == NULL) {
            printf("第 %2d 条弧：", k + 1);
            skipBlank(stdin);   // 跳过空白，寻找下一个可读符号
            scanf("%c", &v1);
            skipBlank(stdin);   // 跳过空白，寻找下一个可读符号
            scanf("%c", &v2);
        } else {
            // 跳过空白，寻找下一个可读符号
            skipBlank(fp);
            ReadData(fp, "%c%c", &v1, &v2);
        }
        
        // 如果需要录入弧的其他附加信息
        if(IncInfo) {
            // 最后录入附加信息
            Input(*G, &info);
        }
        
        // 插入弧<v1, v2>
        InsertArc(G, v1, v2, info);
    }
    
    // 从文件中读取数据时，最后其实应当判断一下是否读到了足够的信息
    return OK;
}

/*
 * 录入弧的相关附加信息
 */
static void Input(OLGraph G, InfoType** info) {
    int weight;
    
    // 在"网"的情形下需要录入权值信息
    if(G.kind == DN || G.kind == UDN) {
        *info = (InfoType*) malloc(sizeof(InfoType));
        
        if(fp == NULL) {
            scanf("%d", &weight);
        } else {
            ReadData(fp, "%d", &weight);
        }
        
        (*info)->weight = weight;
    }
}

/*
 * 查找
 *
 * 返回顶点u在图/网中的位置
 */
int LocateVex(OLGraph G, VertexType u) {
    int i;
    
    for(i = 0; i < G.vexnum; i++) {
        if(G.xlist[i].data == u) {
            return i;
        }
    }
    
    return -1;
}

/*
 * 取值
 *
 * 返回索引v处的顶点值
 */
VertexType GetVex(OLGraph G, int v) {
    if(v < 0 || v >= G.vexnum) {
        return '\0';    // 指定的顶点不存在
    }
    
    return G.xlist[v].data;
}

/*
 * 首个邻接点
 *
 * 返回顶点v的首个邻接点
 */
int FirstAdjVex(OLGraph G, VertexType v) {
    int k;
    ArcBox* r;
    
    // 首先需要判断该顶点是否存在
    k = LocateVex(G, v);
    if(k == -1) {
        return -1;    // 指定的顶点不存在
    }
    
    r = G.xlist[k].firstout;
    if(r == NULL) {
        return -1;
    } else {
        return r->headvex;
    }
}

/*
 * 下一个邻接点
 *
 * 返回顶点v的(相对于w的)下一个邻接点
 */
int NextAdjVex(OLGraph G, VertexType v, VertexType w) {
    int kv, kw;
    ArcBox* r;
    
    // 首先需要判断该顶点是否存在
    kv = LocateVex(G, v);
    if(kv == -1) {
        return -1;    // 指定的顶点不存在
    }
    
    // 首先需要判断该顶点是否存在
    kw = LocateVex(G, w);
    if(kw == -1) {
        return -1;    // 指定的顶点不存在
    }
    
    r = G.xlist[kv].firstout;
    if(r == NULL) {
        return -1;  // 链表为空
    }
    
    // 在链表中查找w
    while(r != NULL && r->headvex < kw) {
        r = r->tlink;
    }
    
    // 如果没找到w
    if(r == NULL) {
        return -1;
    }
    
    // 如果找到了w，但是w后面没有别的顶点，那么也无法返回邻接点
    if(r->headvex == kw && r->tlink != NULL) {
        return r->tlink->headvex;
    }
    
    return -1;
}

/*
 * 构造一个边/弧结点(仅限内部使用)
 */
static ArcBox* newArcBoxPtr(int tailvex, int headvex, ArcBox* hlink, ArcBox* tlink, InfoType* info) {
    ArcBox* p = (ArcBox*) malloc(sizeof(ArcBox));
    if(!p) {
        exit(OVERFLOW);
    }
    
    p->tailvex = tailvex;
    p->headvex = headvex;
    
    p->hlink = hlink;
    p->tlink = tlink;
    
    p->info = info;
    
    return p;
}

/*
 * 插入边/弧<v, w>
 *
 * 如果当前图/网是无向的，则插入一条弧需要增加两个顶点关系，但弧的数量只增一。
 * 对于图/网来说，可以在可变参数中列出边/弧的附加信息。
 *
 * 注：此处接收的参数与MGraph有些不一样：网的附加信息中包含了各条边/弧的权值。
 */
Status InsertArc(OLGraph* G, VertexType v, VertexType w, ...) {
    int tail, head, k, count;
    ArcBox* p;
    ArcBox* pre;
    ArcBox* r;
    Boolean overlay = FALSE;   // 是否为覆盖添加
    InfoType* info = NULL;     // 边/弧的附加信息
    va_list ap;
    
    tail = LocateVex(*G, v); // 获取顶点v在顶点集中的位置
    if(tail == -1) {
        return ERROR;  // 指定的顶点不存在
    }
    
    head = LocateVex(*G, w); // 获取顶点w在顶点集中的位置
    if(head == -1) {
        return ERROR;  // 指定的顶点不存在
    }
    
    // 拒绝环
    if(tail == head) {
        return ERROR;
    }
    
    // 如果边/弧上存在附加信息
    if(IncInfo) {
        va_start(ap, w);                // 在w后查询首个可变参数
        info = va_arg(ap, InfoType*);   // 获取附加信息
        va_end(ap);
    }
    
    /* 接下来，需要查找合适的插入位置 */
    
    for(count = 0; count < 2; count++) {
        
        // 在横向链表上查找合适的插入位置
        pre = NULL;
        // 指向以tail为尾的首条边/弧
        r = G->xlist[tail].firstout;
        while(r != NULL && r->headvex < head) {
            pre = r;
            r = r->tlink;
        }
        
        // 遇到了相同位置的结点
        if(r != NULL && r->headvex == head) {
            r->info = info; // 复用该结点
            overlay = TRUE; // 发生了覆盖
        } else {
            p = newArcBoxPtr(tail, head, NULL, r, info);
            
            if(pre == NULL) {
                G->xlist[tail].firstout = p;
            } else {
                pre->tlink = p;
            }
        }
        
        // 如果没有发生覆盖，说明插入了新结点，此时需要考虑其在纵向链表上的位置
        if(overlay == FALSE) {
            // 在纵向链表上查找合适的插入位置
            pre = NULL;
            // 指向以head为头的首条边/弧
            r = G->xlist[head].firstin;
            while(r != NULL && r->tailvex < tail) {
                pre = r;
                r = r->hlink;
            }
            
            // 遇到了相同位置的结点
            if(r != NULL && r->tailvex == tail) {
                // 不会执行到这里，因为如果发生了覆盖，前面进不来
            } else {
                /* 至此，结点p已经存在了 */
                
                if(pre == NULL) {
                    p->hlink = G->xlist[head].firstin;
                    G->xlist[head].firstin = p;
                } else {
                    p->hlink = pre->hlink;
                    pre->hlink = p;
                }
            }
        }
        
        // 如果当前图/网是无向的，需要考虑对称性
        if((G->kind == UDG || G->kind == UDN) && tail != head) {
            // 颠倒i和j
            k = tail;
            tail = head;
            head = k;
        } else {
            break;  // 如果是有向的，可以结束了
        }
    }
    
    // 在非覆盖的情形下，才考虑更新边/弧的数量
    if(!overlay) {
        (*G).arcnum++;  // 不论有向无向，边/弧数只增一
    }
    
    return OK;
}

/*
 * 深度优先遍历(此处借助递归实现)
 */
void DFSTraverse(OLGraph G, Status(Visit)(VertexType)) {
    int v;
    
    // 使用全局变量VisitFunc，使得DFS不必设置函数指针参数
    VisitFunc = Visit;
    
    // 访问标志数组初始化
    for(v = 0; v < G.vexnum; v++) {
        visited[v] = FALSE;
    }
    
    // 此处需要遍历的原因是并不能保证所有顶点都连接在了一起
    for(v = 0; v < G.vexnum; v++) {
        if(!visited[v]) {
            DFS(G, v);  // 对尚未访问的顶点调用DFS
        }
    }
}

/*
 * 深度优先遍历核心函数
 */
static void DFS(OLGraph G, int v) {
    int w;
    
    // 从第v个顶点出发递归地深度优先遍历图G
    visited[v] = TRUE;
    
    // 访问第v个顶点
    VisitFunc(G.xlist[v].data);
    
    for(w = FirstAdjVex(G, G.xlist[v].data);
        w >= 0;
        w = NextAdjVex(G, G.xlist[v].data, G.xlist[w].data)) {
        if(!visited[w]) {
            DFS(G, w);  // 对尚未访问的顶点调用DFS
        }
    }
}

/*
 * 以图形化形式输出当前结构
 */
void PrintGraph(OLGraph G) {
    int i, head;
    ArcBox* p;
    
    if(G.vexnum == 0) {
        printf("空图，无需打印！\n");
        return;
    }
    
    printf("当前图/网包含 %2d 个顶点， %2d 条边/弧...\n", G.vexnum, G.arcnum);
    
    for(i = 0; i < G.vexnum; i++) {
        printf("%c ===> ", G.xlist[i].data);
        
        head = 0;
        p = G.xlist[i].firstout;
        
        while(p != NULL) {
            if(head < p->headvex) {
                if(IncInfo == 0) {
                    printf("      ");
                    
                    // 对于网，会从其附加信息中获取到权值
                } else {
                    printf("          ");
                }
            } else {
                if(IncInfo == 0) {
                    printf("<%c, %c>", G.xlist[p->tailvex].data, G.xlist[p->headvex].data);
                    
                    // 对于网，会从其附加信息中获取到权值
                } else {
                    printf("<%c, %c, %2d>", G.xlist[p->tailvex].data, G.xlist[p->headvex].data, p->info->weight);
                }
                
                p = p->tlink;
            }
            
            head++;
            
            if(p != NULL) {
                printf("  ");
            }
        }
        
        printf("\n");
    }
}

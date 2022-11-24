typedef void *Pointer; //Void pointer to store any kind of data needed
typedef void(*DestroyFunc)(Pointer); //Function to propertly free the momory that our data are using
typedef int(*CompareFunc)(Pointer a,Pointer b);//Returns 0 if a=b,<0 if a<b,>0 if a>b

typedef struct _list *ListPtr;

ListPtr CreateList(CompareFunc compare ,DestroyFunc destroy);
int ListEmpty(ListPtr ls);
void ListInsert(Pointer Item, ListPtr ls);
Pointer ListRemove_nth(ListPtr ls, int N);
int ListFind(ListPtr ls, Pointer value);
void freeList(ListPtr ls);
Pointer ListGetNth(ListPtr ls, int N);
int ListSize(ListPtr ls);
void ListToArray(ListPtr ls, Pointer* array);

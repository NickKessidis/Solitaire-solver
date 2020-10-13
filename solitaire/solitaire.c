#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <time.h>

#define N 13

#define HEARTS 0        //constants denoting the four card types
#define DIAMONDS 1
#define SPADES 2
#define CLUBS 3

#define freec    0      //constants denoting the four directions
#define stak     1
#define newstack 2
#define found    3

#define breadth 1		// Constants denoting the four algorithms
#define depth	2
#define best	3
#define astar	4

typedef struct card{
	int suit;
	int value;
}card;

struct card* Create_card(int s, int v){
struct card *new_card = (struct card*)malloc(sizeof(struct card));
new_card->suit = s;
new_card->value = v;
return new_card;
};

clock_t t1;					// Start time of the search algorithm
clock_t t2;					// End time of the search algorithm
#define TIMEOUT		300

typedef struct{
    int top;
    struct card P[4*N];
}Stack;

struct tree_node{
    int h;              // the value of the heuristic function for this node
	int g;						// the depth of this node wrt the root of the search tree
	int f;                  // f=0 or f=h or f=h+g, depending on the search algorithm used.
	struct tree_node *parent;	// pointer to the parrent node (NULL for the root).
	int direction;
	struct card C;
    card freecell[4];
    Stack stack[8];
    Stack foundation[4];
}tree_node;

struct frontier_node
{
	struct tree_node *n;				// pointer to a search-tree node
	struct frontier_node *previous;		// pointer to the previous frontier node
	struct frontier_node *next;			// pointer to the next frontier node
};

struct frontier_node *frontier_head=NULL;	// The one end of the frontier
struct frontier_node *frontier_tail=NULL;	// The other end of the frontier

card *cards;
int solution_length;	// The lenght of the solution table.
int *directions;	        // Pointer to a dynamic table with the moves of the solution.

struct tree_node* Create_tree_node()
{
    int i;
    struct tree_node *new_node = malloc(sizeof (tree_node));
    if(new_node == NULL)
    {
        return NULL;
    }
    else
    {
        new_node->C.suit = -1;
        new_node->C.value = -1;
        new_node->direction = -1;
        new_node->f = 0;
        new_node->g = 0;
        new_node->h = 0;
        new_node->parent = NULL;
        for(i=0;i<8;i++)
        {
            new_node->stack[i].top=-1;
        }
        for(i=0;i<4;i++)
        {
            new_node->foundation[i].top=-1;
            new_node->freecell[i].suit=-1;
            new_node->freecell[i].value=-1;
        }
        return new_node;
    }
}

struct frontier_node* Create_frontier_node()
{
    int i;
    struct frontier_node *new_frontier_node=malloc(sizeof(struct frontier_node));
    if(new_frontier_node==NULL)
    {
        return NULL;
    }
    else
    {
        new_frontier_node->next = NULL;
        new_frontier_node->previous = NULL;
        new_frontier_node->n = Create_tree_node();
        new_frontier_node->n->C.suit = -1;
        new_frontier_node->n->C.value = -1;
        new_frontier_node->n->direction = -1;
        new_frontier_node->n->f = 0;
        new_frontier_node->n->g = 0;
        new_frontier_node->n->h = 0;
        new_frontier_node->n->parent = NULL;
        for(i=0;i<8;i++)
        {
            new_frontier_node->n->stack[i].top=-1;
        }
        for(i=0;i<4;i++)
        {
            new_frontier_node->n->foundation[i].top=-1;
            new_frontier_node->n->freecell[i].suit=-1;
            new_frontier_node->n->freecell[i].value=-1;
        }
        return new_frontier_node;
    }
}

void Create_Stack(Stack *s)
{
    s->top = -1;
}

int get_method(char* s)
{
	if (strcmp(s,"breadth")==0)
		return  breadth;
	else if (strcmp(s,"depth")==0)
		return depth;
	else if (strcmp(s,"best")==0)
		return best;
	else if (strcmp(s,"astar")==0)
		return astar;
	else
		return -1;
}

void display_stack(Stack s)
{
	int i;

	if(s.top==-1)
	{
		printf("\nStack is empty!!");
	}
	else
	{
		printf("\nStack is...\n");
		for(i=s.top;i>=0;--i)
			printf("%d %d\n",s.P[i].suit, s.P[i].value);
	}
}

void Push(Stack *s, struct card item)
{
    s->top++;
    s->P[s->top].suit = item.suit;
    s->P[s->top].value = item.value;
}

int isEmpty(Stack s)
{
    if (s.top==-1)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void syntax_message()
{
	printf("puzzle <method> <input-file> <output-file>\n\n");
	printf("where: ");
	printf("<method> = breadth|depth|best|astar\n");
	printf("<input-file> is a file containing a %dx%d puzzle description.\n",N,N);
	printf("<output-file> is the file where the solution will be written.\n");
}

void Pop(Stack *s, struct card *item)
{
    if(isEmpty(*s)==0)
    {
        *item = s->P[s->top];
        s->top--;
    }
}

void stacks_equal(Stack q, Stack *z)
{
    int i,j;
    if(isEmpty(q)!=-1)
    {
        Stack temp;
        Create_Stack(&temp);

        for(i=q.top;i>=0;i--)
        {
            Push(&temp, q.P[i]);
        }
        for(j=q.top;j>=0;j--)
        {
            Push(z, temp.P[j]);
        }
    }
}

int read_deck(char* filename, Stack st[])
{
	FILE *fin;
	char temp;
	int i,j,err,n,s;
	fin=fopen(filename, "r");
	if (fin==NULL)
	{
		#ifdef SHOW_COMMENTS
			printf("Cannot open file %s. Program terminates.\n",filename);
		#endif
		return -1;
	}
    i = 0;
    err=fscanf(fin,"%c",&temp);
	while(err!=EOF)
		{
			if (err<1)
			{
				#ifdef SHOW_COMMENTS
					printf("Cannot read item [%d][%d] of the puzzle. Program terminates.\n",i,j);
				#endif
				fclose(fin);
				return -1;
			}

            switch (temp)
            {
            case 'H':
                s=0;
                break;
            case 'D':
                s=1;
                break;
            case 'S':
                s=2;
                break;
            case 'C':
                s=3;
                break;
            }
            err=fscanf(fin,"%d",&n);
            //printf("%d %d\n", s, n);
            struct card *c = Create_card(s,n);
            Push(&st[i], *c);
            err=fscanf(fin,"%c",&temp);
            if(temp == 10)
            {
                i++;
                err=fscanf(fin,"%c",&temp);
            }
            else if (temp == 32)
            {
                err=fscanf(fin,"%c",&temp);
            }
		}
	fclose(fin);
	return 0;
}

int add_frontier_front(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=Create_frontier_node();
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->previous=NULL;
	new_frontier_node->next=frontier_head;

	if (frontier_head==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		frontier_head->previous=new_frontier_node;
		frontier_head=new_frontier_node;
	}

#ifdef SHOW_COMMENTS
	printf("Added to the front...\n");
	display_puzzle(node->p);
#endif
	return 0;
}


void initialize_search(Stack st[], int method)
{
	struct tree_node *root=NULL;	// the root of the search tree.
	int i,j,jj;
    Stack s;
    Create_Stack(&s);
	// Initialize search tree
	root=Create_tree_node();
	root->parent=NULL;
	root->direction=-1;
	root->C.suit = -1;
	root->C.value = -1;
	for(i=0;i<8;i++)
    {
        root->stack[i] = st[i];
        //stacks_equal(st[i], &root->stack[i]);
        //display_stack(root->stack[i]);
    }


    for(i=0;i<4;i++)
    {
        root->freecell[i].suit = -1;
        root->freecell[i].value = -1;
        root->foundation[i] = s;
    }
	root->g=0;
	root->h=0;
	//root->h=heuristic(root->p);
	if (method==best)
		root->f=root->h;
	else if (method==astar)
		root->f=root->g+root->h;
	else
		root->f=0;

	// Initialize frontier
	add_frontier_front(root);
}

int cards_equal(struct card c1,struct  card c2)
{
    if((c1.suit==c2.suit)&&(c1.value==c2.value))
    {
        return 0;
    }
    else return -1;
}

int add_frontier_back(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=Create_frontier_node();
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->next=NULL;
	new_frontier_node->previous=frontier_tail;

	if (frontier_tail==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		frontier_tail->next=new_frontier_node;
		frontier_tail=new_frontier_node;
	}

#ifdef SHOW_COMMENTS
	printf("Added to the back...\n");
	display_puzzle(node->p);
#endif

	return 0;
}

int add_frontier_in_order(struct tree_node *node)
{

	// Creating the new frontier node
	struct frontier_node *new_frontier_node=Create_frontier_node();
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->previous=NULL;
	new_frontier_node->next=NULL;

	if (frontier_head==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		struct frontier_node *pt;
		pt=frontier_head;

		// Search in the frontier for the first node that corresponds to either a larger f value
		// or to an equal f value but larger h value
		// Note that for the best first search algorithm, f and h values coincide.
		while (pt!=NULL && (pt->n->f<node->f || (pt->n->f==node->f && pt->n->h<node->h)))
			pt=pt->next;

		if (pt!=NULL)
		{
			// new_frontier_node is inserted before pt .
			if (pt->previous!=NULL)
			{
				pt->previous->next=new_frontier_node;
				new_frontier_node->next=pt;
				new_frontier_node->previous=pt->previous;
				pt->previous=new_frontier_node;
			}
			else
			{
				// In this case, new_frontier_node becomes the first node of the frontier.
				new_frontier_node->next=pt;
				pt->previous=new_frontier_node;
				frontier_head=new_frontier_node;
			}
		}
		else
		{
			// if pt==NULL, new_frontier_node is inserted at the back of the frontier
			frontier_tail->next=new_frontier_node;
			new_frontier_node->previous=frontier_tail;
			frontier_tail=new_frontier_node;
		}
	}

#ifdef SHOW_COMMENTS
	printf("Added in order (f=%d)...\n",node->f);
	display_puzzle(node->p);
#endif

	return 0;
}

int are_equal(Stack s1[], Stack s2[], Stack f1[], Stack f2[])
{
    int i, j, z;
    for(i=0;i<8;i++)
    {
        if(s1[i].top==s2[i].top)
        {
            j = s1[i].top;
            while(j != -1)
            {
                if(cards_equal(s1[i].P[j], s2[i].P[j])==-1)
                {
                    return -1;
                }
                j--;
            }
        }
        else return -1;

        if(i<4)
        {
            if(f1[i].top==f2[i].top)
            {
                z = f1[i].top;
                while(z != -1)
                {
                    if(cards_equal(f1[i].P[z], f2[i].P[z])==-1)
                    {
                        return -1;
                    }
                    z--;
                }
            }
            else return -1;
        }

    }
    return 0;
}

int sum_of(Stack s[])
{
    int i,j;
    int c=0;
    for(i=0;i<4;i++)           //allagh se 4 meta!!!
    {
        if(s[i].top>=0)
        {
            for(j=s[i].top;j>=0;j--)
            {
                c++;
            }
        }
    }
    return c;
}

int heuristic(Stack s[])
{
    return (4*N)-sum_of(s);
}

int check_with_parents(struct tree_node *new_node)
{
	struct tree_node *parent = Create_tree_node();
	parent=new_node->parent;
	while (parent!=NULL)
	{
		if (are_equal(new_node->stack, parent->stack, new_node->foundation, parent->foundation)==0)
			return 0;
		parent=parent->parent;
	}
	return 1;
}

void freecell_equal(struct card A, struct card* B)
{
    struct card temp;

    temp.suit = A.suit;
    temp.value = A.value;

    B->suit = temp.suit;
    B->value = temp.value;
}

int find_children(struct tree_node *current_node, int method)
{
    int i,j, z;

    //check for direction: stack->foundation
    for(i=0;i<8;i++)
    {
        int flag1 = 0;

        if(isEmpty(current_node->stack[i])==0)
        {
            if(isEmpty(current_node->foundation[current_node->stack[i].P[current_node->stack[i].top].suit])==-1)
            {
                if(current_node->stack[i].P[current_node->stack[i].top].value==0)
                {
                    flag1 = 1;
                }
            }
            else
            {
                if(current_node->stack[i].P[current_node->stack[i].top].value == (current_node->foundation[current_node->stack[i].P[current_node->stack[i].top].suit].P[current_node->foundation[current_node->stack[i].P[current_node->stack[i].top].suit].top].value + 1))
                {
                    flag1 = 1;
                }
            }
        }


        if(flag1 == 1)
        {
            struct card *c = Create_card(0,0);
            struct tree_node *child=Create_tree_node();
            if (child==NULL) return -1;

            for(z=0;z<8;z++)
            {
                stacks_equal(current_node->stack[z], &child->stack[z]);
            }
            for(z=0;z<4;z++)
            {
                stacks_equal(current_node->foundation[z], &child->foundation[z]);
                freecell_equal(current_node->freecell[z], &child->freecell[z]);
            }
            child->parent=current_node;
            child->direction=found;
            child->g=current_node->g+1;
            Pop(&child->stack[i], c);
            switch (c->suit)
            {
            case 0:
                Push(&child->foundation[0], *c);
                break;
            case 1:
                Push(&child->foundation[1], *c);
                break;
            case 2:
                Push(&child->foundation[2], *c);
                break;
            case 3:
                Push(&child->foundation[3], *c);
                break;
            }
            child->C.suit = c->suit;
            child->C.value = c->value;

            // Check for loops
            if (check_with_parents(child)==0)
            {
                // In case of loop detection, the child is deleted
                free(child);
            }
            else
            {
                // Computing the heuristic value
                child->h=heuristic(child->foundation);
                if (method==best)
                    child->f=child->h;
                else if (method==astar)
                    child->f=child->g+child->h;
                else
                    child->f=0;

                int err=0;
                if (method==depth)
                    err=add_frontier_front(child);
                else if (method==breadth)
                    err=add_frontier_back(child);
                else if (method==best || method==astar)
                    err=add_frontier_in_order(child);
                if (err<0)
                    return -1;
            }

        }
    }
    //check for direction: freecell->foundation
    for(i=0;i<4;i++)
    {
        int flag=0;
        if(current_node->freecell[i].suit!=-1)
        {
            if(isEmpty(current_node->foundation[current_node->freecell[i].suit])==-1)
            {
                if(current_node->freecell[i].value==0) flag = 1;
            }
            else
            {
                if(current_node->freecell[i].value == (current_node->foundation[current_node->freecell[i].suit].P[current_node->foundation[current_node->freecell[i].suit].top].value + 1)) flag = 1;
            }
            if(flag = 1)
            {
                struct card *c = Create_card(0,0);
                struct tree_node *child=Create_tree_node();
                if (child==NULL) return -1;

                for(z=0;z<8;z++)
                {
                    stacks_equal(current_node->stack[z], &child->stack[z]);
                }
                for(z=0;z<4;z++)
                {
                    stacks_equal(current_node->foundation[z], &child->foundation[z]);
                    freecell_equal(current_node->freecell[z], &child->freecell[z]);
                }
                child->parent=current_node;
                child->direction=found;
                child->g=current_node->g+1;
                c->suit = current_node->freecell[i].suit;
                c->value = current_node->freecell[i].value;
                child->freecell[i].suit = -1;
                child->freecell[i].value = -1;
                switch (c->suit)
                {
                case 0:
                    Push(&child->foundation[0], *c);
                    break;
                case 1:
                    Push(&child->foundation[1], *c);
                    break;
                case 2:
                    Push(&child->foundation[2], *c);
                    break;
                case 3:
                    Push(&child->foundation[3], *c);
                    break;
                }
                child->C.suit = c->suit;
                child->C.value = c->value;

                // Check for loops
                if (check_with_parents(child)==0)
                {
                    // In case of loop detection, the child is deleted
                    free(child);
                }
                else
                {
                    // Computing the heuristic value
                    child->h=heuristic(child->foundation);
                    if (method==best)
                        child->f=child->h;
                    else if (method==astar)
                        child->f=child->g+child->h;
                    else
                        child->f=0;

                    int err=0;
                    if (method==depth)
                        err=add_frontier_front(child);
                    else if (method==breadth)
                        err=add_frontier_back(child);
                    else if (method==best || method==astar)
                        err=add_frontier_in_order(child);
                    if (err<0)
                        return -1;
                }
            }
        }
    }

    //check for direction  stack->stack
    for(i=0;i<7;i++)
    {
        int flag1 = 0;
        int flag2 = 0;
        int flag3 = 0;
        int topi = current_node->stack[i].top;
        for(j=i+1;j<8;j++)
        {
            if(isEmpty(current_node->stack[i])==-1 && isEmpty(current_node->stack[j])!=-1)
            {
                flag2 = 1;
                flag3 = 1;
            }
            else if (isEmpty(current_node->stack[j])==-1 && isEmpty(current_node->stack[i])!=-1)
            {
                flag1 = 1;
                flag3 = 1;
            }
            int topj = current_node->stack[j].top;
            if(isEmpty(current_node->stack[i])!=-1)
            {
                int suitI = current_node->stack[i].P[topi].suit;
                int suitJ = current_node->stack[j].P[topj].suit;
                if(((suitI == HEARTS || suitI == DIAMONDS) && (suitJ == SPADES || suitJ == CLUBS)) || ((suitI == SPADES || suitI == CLUBS) && (suitJ == HEARTS || suitJ == DIAMONDS)))
                {
                    if(current_node->stack[i].P[topi].value +1 == current_node->stack[j].P[topj].value)
                    {
                        flag1 = 1;
                    }
                }
                if(isEmpty(current_node->stack[j])!=-1)
                {
                    if(((suitI == HEARTS || suitI == DIAMONDS) && (suitJ == SPADES || suitJ == CLUBS)) || ((suitI == SPADES || suitI == CLUBS) && (suitJ == HEARTS || suitJ == DIAMONDS)))
                    {
                        if(current_node->stack[j].P[topj].value +1 == current_node->stack[i].P[topi].value)
                        {
                            flag2 = 1;
                        }
                    }
                }
            }

            if(flag1==1)
            {
                struct card *c = Create_card(0,0);
                struct tree_node *child=Create_tree_node();
                if (child==NULL) return -1;

                for(z=0;z<8;z++)
                {
                    stacks_equal(current_node->stack[z], &child->stack[z]);
                }
                for(z=0;z<4;z++)
                {
                    stacks_equal(current_node->foundation[z], &child->foundation[z]);
                    freecell_equal(current_node->freecell[z], &child->freecell[z]);
                }
                child->parent=current_node;
                if(flag3==1)
                {
                    child->direction=newstack;
                }
                else
                {
                    child->direction=stak;
                }
                child->g=current_node->g+1;
                Pop(&child->stack[i], c);
                Push(&child->stack[j], *c);
                child->C.suit = c->suit;
                child->C.value = c->value;

                // Check for loops
                if (check_with_parents(child)==0)
                {
                    // In case of loop detection, the child is deleted
                    free(child);
                }
                else
                {
                    // Computing the heuristic value
                    child->h=heuristic(child->foundation);
                    if (method==best)
                        child->f=child->h;
                    else if (method==astar)
                        child->f=child->g+child->h;
                    else
                        child->f=0;

                    int err=0;
                    if (method==depth)
                        err=add_frontier_front(child);
                    else if (method==breadth)
                        err=add_frontier_back(child);
                    else if (method==best || method==astar)
                        err=add_frontier_in_order(child);
                    if (err<0)
                        return -1;
                }
            }
            if(flag2==1)
            {
                struct card *c = Create_card(0,0);
                struct tree_node *child=Create_tree_node();
                if (child==NULL) return -1;

                for(z=0;z<8;z++)
                {
                    stacks_equal(current_node->stack[z], &child->stack[z]);
                }
                for(z=0;z<4;z++)
                {
                    stacks_equal(current_node->foundation[z], &child->foundation[z]);
                    freecell_equal(current_node->freecell[z], &child->freecell[z]);
                }

                child->parent=current_node;
                if(flag3==1)
                {
                    child->direction=newstack;
                }
                else
                {
                    child->direction=stak;
                }
                child->g=current_node->g+1;
                Pop(&child->stack[j], c);
                Push(&child->stack[i], *c);
                child->C.suit = c->suit;
                child->C.value = c->value;

                // Check for loops
                if(check_with_parents(child)==0)
                {
                    // In case of loop detection, the child is deleted
                    free(child);
                }
                else
                {
                    // Computing the heuristic value
                    child->h=heuristic(child->foundation);
                    if (method==best)
                        child->f=child->h;
                    else if (method==astar)
                        child->f=child->g+child->h;
                    else
                        child->f=0;

                    int err=0;
                    if (method==depth)
                        err=add_frontier_front(child);
                    else if (method==breadth)
                        err=add_frontier_back(child);
                    else if (method==best || method==astar)
                        err=add_frontier_in_order(child);
                    if (err<0)
                        return -1;
                }
            }
        }
    }
    //check for direction freecell->stack
    for(i=0;i<4;i++)
    {
        int flag =0;
        int suitF = current_node->freecell[i].suit;
        if(suitF!=-1)
        {
            for(j=0;j<8;j++)
            {
                if(isEmpty(current_node->stack[j])!=-1)
                {
                    int suitS = current_node->stack[j].P[current_node->stack[j].top].suit;
                    if(((suitF == HEARTS || suitF == DIAMONDS) && (suitS == SPADES || suitS == CLUBS)) || ((suitF == SPADES || suitF == CLUBS) && (suitS == HEARTS || suitS == DIAMONDS)))
                    {
                        if(current_node->freecell[i].value +1 == current_node->stack[j].P[current_node->stack[j].top].value)
                        {
                           flag = 1;
                        }
                    }
                }
                else
                {
                   flag = 2;
                }
            }
        }
        if(flag!=0)
        {
            struct card *c = Create_card(0,0);
            struct tree_node *child=Create_tree_node();
            if (child==NULL) return -1;

            for(z=0;z<8;z++)
            {
                stacks_equal(current_node->stack[z], &child->stack[z]);
            }
            for(z=0;z<4;z++)
            {
                stacks_equal(current_node->foundation[z], &child->foundation[z]);
                freecell_equal(current_node->freecell[z], &child->freecell[z]);
            }
            child->parent=current_node;
            if(flag==1)
            {
                child->direction=stak;
            }
            else if(flag==2)
            {
                child->direction=newstack;
            }
            child->g=current_node->g+1;
            //Pop(&child->freecell[i], c);
            c->suit = child->freecell[i].suit;
            c->value = child->freecell[i].value;
            child->freecell[i].suit = -1;
            child->freecell[i].value = -1;
            Push(&child->stack[j], *c);
            child->C.suit = c->suit;
            child->C.value = c->value;

            // Check for loops
            if (check_with_parents(child)==0)
            {
                // In case of loop detection, the child is deleted
                free(child);
            }
            else
            {
            // Computing the heuristic value
            child->h=heuristic(child->foundation);
            if (method==best)
                child->f=child->h;
            else if (method==astar)
                child->f=child->g+child->h;
            else
                child->f=0;

            int err=0;
            if (method==depth)
                err=add_frontier_front(child);
            else if (method==breadth)
                err=add_frontier_back(child);
            else if (method==best || method==astar)
                err=add_frontier_in_order(child);
            if (err<0)
                return -1;
            }
        }

    }

    //check for direction stack->freecell
    for(i=0;i<8;i++)
    {
        if(isEmpty(current_node->stack[i])!=-1)
        {
            for(j=0;j<4;j++)
            {
                if(current_node->freecell[j].suit==-1)
                {
                    struct card *c = Create_card(0,0);
                    struct tree_node *child=Create_tree_node();
                    if (child==NULL) return -1;

                    for(z=0;z<8;z++)
                    {
                        stacks_equal(current_node->stack[z], &child->stack[z]);
                    }
                    for(z=0;z<4;z++)
                    {
                        stacks_equal(current_node->foundation[z], &child->foundation[z]);
                        freecell_equal(current_node->freecell[z], &child->freecell[z]);
                    }
                    child->parent=current_node;
                    child->direction=freec;
                    child->g=current_node->g+1;
                    Pop(&child->stack[i], c);
                    child->freecell[j].suit = c->suit;
                    child->freecell[j].value = c->value;
                    child->C.suit = c->suit;
                    child->C.value = c->value;

                    // Check for loops
                    if (check_with_parents(child)==0)
                    {
                        // In case of loop detection, the child is deleted
                        free(child);
                    }
                    else
                    {
                        // Computing the heuristic value
                        child->h=heuristic(child->foundation);
                        if (method==best)
                            child->f=child->h;
                        else if (method==astar)
                            child->f=child->g+child->h;
                        else
                            child->f=0;

                        int err=0;
                        if (method==depth)
                            err=add_frontier_front(child);
                        else if (method==breadth)
                            err=add_frontier_back(child);
                        else if (method==best || method==astar)
                            err=add_frontier_in_order(child);
                        if (err<0)
                            return -1;
                    }
                }
            }
        }
    }

    //check for direction foundation -> stack
    for(i=0;i<4;i++)
    {
        int flag;
        if(isEmpty(current_node->foundation[i])!=-1)
        {
            for(j=0;j<8;j++)
            {
                flag = 0;
                if(isEmpty(current_node->stack[j])==-1)
                {
                    flag = 1;
                }
                else
                {
                    int suitF = current_node->foundation[i].P[current_node->foundation[i].top].suit;
                    int suitS = current_node->stack[j].P[current_node->stack[j].top].suit;
                    if(((suitF == HEARTS || suitF == DIAMONDS) && (suitS == SPADES || suitS == CLUBS)) || ((suitF == SPADES || suitF == CLUBS) && (suitS == HEARTS || suitS == DIAMONDS)))
                    {
                        if(current_node->foundation[i].P[current_node->foundation[i].top].value +1 == current_node->stack[j].P[current_node->stack[j].top].value)
                        {
                           flag = 2;
                        }
                    }
                }
                if(flag!=0)
                {
                    struct card *c = Create_card(0,0);
                    struct tree_node *child=Create_tree_node();
                    if (child==NULL) return -1;
                    for(z=0;z<8;z++)
                    {
                        stacks_equal(current_node->stack[z], &child->stack[z]);
                    }
                    for(z=0;z<4;z++)
                    {
                        stacks_equal(current_node->foundation[z], &child->foundation[z]);
                        freecell_equal(current_node->freecell[z], &child->freecell[z]);
                    }
                    child->parent=current_node;
                    if(flag==1)
                    {
                        child->direction=newstack;
                    }
                    else
                    {
                        child->direction=stak;
                    }
                    child->g=current_node->g+1;
                    Pop(&child->foundation[i], c);
                    Push(&child->stack[j], *c);
                    child->C.suit = c->suit;
                    child->C.value = c->value;

                    // Check for loops
                    if (check_with_parents(child)==0)
                    {
                        // In case of loop detection, the child is deleted
                        free(child);
                    }
                    else
                    {
                        // Computing the heuristic value
                        child->h=heuristic(child->foundation);
                        if (method==best)
                            child->f=child->h;
                        else if (method==astar)
                            child->f=child->g+child->h;
                        else
                            child->f=0;

                        int err=0;
                        if (method==depth)
                            err=add_frontier_front(child);
                        else if (method==breadth)
                            err=add_frontier_back(child);
                        else if (method==best || method==astar)
                            err=add_frontier_in_order(child);
                        if (err<0)
                            return -1;
                    }
                }
            }
        }
    }

    //check for direction foundation->freecell
    for(i=0;i<4;i++)
    {
        if(isEmpty(current_node->foundation[i])!=-1)
        {
            for(j=0;j<4;j++)
            {
                if(current_node->freecell[j].suit==-1)
                {
                    struct card *c = Create_card(0,0);
                    struct tree_node *child=Create_tree_node();
                    if (child==NULL) return -1;

                    for(z=0;z<8;z++)
                    {
                        stacks_equal(current_node->stack[z], &child->stack[z]);
                    }
                    for(z=0;z<4;z++)
                    {
                        stacks_equal(current_node->foundation[z], &child->foundation[z]);
                        freecell_equal(current_node->freecell[z], &child->freecell[z]);
                    }
                    child->parent=current_node;
                    child->direction=freec;
                    child->g=current_node->g+1;
                    Pop(&child->foundation[i], c);
                    child->freecell[j].suit = c->suit;
                    child->freecell[j].value = c->value;
                    child->C.suit = c->suit;
                    child->C.value = c->value;

                    // Check for loops
                    if (check_with_parents(child)==0)
                    {
                        // In case of loop detection, the child is deleted
                        free(child);
                    }
                    else
                    {
                        // Computing the heuristic value
                        child->h=heuristic(child->foundation);
                        if (method==best)
                            child->f=child->h;
                        else if (method==astar)
                            child->f=child->g+child->h;
                        else
                            child->f=0;

                        int err=0;
                        if (method==depth)
                            err=add_frontier_front(child);
                        else if (method==breadth)
                            err=add_frontier_back(child);
                        else if (method==best || method==astar)
                            err=add_frontier_in_order(child);
                        if (err<0)
                            return -1;
                    }
                }
            }
        }
    }

    return 1;
}

int is_solution(Stack f[])
{
	if(sum_of(f)>=N)
	{
        return 0;
	}
	return 1;
}

struct tree_node *search(int method)
{
	clock_t t;
	int i, err, j;
	struct frontier_node *temp_frontier_node = Create_frontier_node();
	struct tree_node *current_node = Create_tree_node();

	i=0;
	while (frontier_head!=NULL)
	{
	    printf("\n---------i=%d-----------\n", i);
		t=clock();
		if (t-t1>CLOCKS_PER_SEC*TIMEOUT)
		{
			printf("Timeout\n");
			return NULL;
		}

		// Extract the first node from the frontier

		current_node=frontier_head->n;

        display_stack(frontier_head->n->foundation[0]);


		if (is_solution(current_node->foundation)==0)
			return current_node;
		// Delete the first node of the frontier
		temp_frontier_node=frontier_head;
		frontier_head=frontier_head->next;
		free(temp_frontier_node);
		if (frontier_head==NULL)
			frontier_tail=NULL;
		else
			frontier_head->previous=NULL;


		// Find the children of the extracted node
		int err=find_children(current_node, method);


		if (err<0)
        {
            printf("Memory exhausted while creating new frontier node. Search is terminated...\n");
			return NULL;
        }
        i++;
	}

	return NULL;
}

void extract_solution(struct tree_node *solution_node)
{
	int i;

	struct tree_node *temp_node=solution_node;
	solution_length=solution_node->g;

	directions= (int*) malloc(solution_length*sizeof(int));
	temp_node=solution_node;
	i=solution_length;
	while (temp_node->parent!=NULL)
	{
		i--;
		directions[i]=temp_node->direction;
		cards[i].suit = temp_node->C.suit;
		cards[i].value = temp_node->C.value;
		temp_node=temp_node->parent;
	}
}

void write_solution_to_file(char* filename, int solution_length, int *solution, card *Cards)
{
	int i;
	FILE *fout;
	fout=fopen(filename,"w");
	if (fout==NULL)
	{
		printf("Cannot open output file to write solution.\n");
		printf("Now exiting...");
		return;
	}
	fprintf(fout,"%d\n",solution_length);
	for (i=0;i<solution_length;i++)
    {
		switch(solution[i])
		{
		case freec:
			fprintf(fout,"freecell ");
			break;
		case stak:
			fprintf(fout,"stack ");
			break;
		case newstack:
			fprintf(fout,"newstack ");
			break;
		case found:
			fprintf(fout,"foundation ");
			break;
        }

        switch(Cards[i].suit)
        {
        case HEARTS:
            fprintf(fout,"H %d\n", Cards[i].value);
            break;
        case DIAMONDS:
            fprintf(fout,"D %d\n", Cards[i].value);
            break;
        case SPADES:
            fprintf(fout,"S %d\n", Cards[i].value);
            break;
        case CLUBS:
            fprintf(fout,"C %d\n", Cards[i].value);
            break;
        }
    }
	fclose(fout);
}

int main(int argc, char** argv)
{
    Stack s;
    Create_Stack(&s);
    int err, i;
	struct tree_node *solution_node = Create_tree_node();
	Stack st[8];		            // The initial stack read from a file
    int method;                     // The search algorithm that will be used.

    for(i=0;i<8;i++)
    {
        st[i] = s;
    }

	if (argc!=3)
	{
		printf("Wrong number of arguments. Use correct syntax:\n");
		syntax_message();
		return -1;
	}

	method=get_method(argv[1]);
	if (method<0)
	{
		printf("Wrong method. Use correct syntax:\n");
		syntax_message();
		return -1;
	}

	err=read_deck(argv[2], st);
	if (err<0)
		return -1;

	printf("Solving %s using %s...\n",argv[2],argv[1]);

	t1=clock();

	initialize_search(st, method);

	solution_node=search(method);			// The main call

	t2=clock();

	if (solution_node!=NULL)
		extract_solution(solution_node);
	else
		printf("No solution found.\n");

	if (solution_length>0)
	{
		printf("Solution found! (%d steps)\n",solution_length);
		printf("Time spent: %f secs\n",((float) t2-t1)/CLOCKS_PER_SEC);
		write_solution_to_file(argv[3], solution_length, directions, cards);
	}
    return 0;
}

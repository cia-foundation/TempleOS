#define N	32
 
class Node
{
  Node *left,*right;
  I64 n;
};
 
I64 n1,n2,common_ancestor;
Node *root;
 
#define X_SPACING	16
#define Y_SPACING	45
#define ARROW_SPACING	3
 
U0 ShowTree(CDC *dc,Node *tmpn,I64 *_node_x,I64 *_tree_x,I64 y)
{
  I64 node_x;
  if (tmpn) {
    if (tmpn->left) {
      ShowTree(dc,tmpn->left,&node_x,_tree_x,y+Y_SPACING);
      dc->color=BLUE;
      GrArrow3(dc,*_tree_x,y,0,
	    node_x+ARROW_SPACING,y+Y_SPACING-ARROW_SPACING,0);
    }
    if (tmpn->n==n1 || tmpn->n==n2) {
      if (tmpn->n==common_ancestor)
	dc->color=YELLOW;
      else
	dc->color=RED;
    } else if (tmpn->n==common_ancestor)
      dc->color=GREEN;
    else
      dc->color=BLUE;

    *_node_x=*_tree_x;
    GrPrint(dc,*_node_x,y,"%d",tmpn->n);
    *_tree_x+=X_SPACING;

    if (tmpn->right) {
      ShowTree(dc,tmpn->right,&node_x,_tree_x,y+Y_SPACING);
      dc->color=BLUE;
      GrArrow3(dc,*_node_x,y,0,
	    node_x-ARROW_SPACING,y+Y_SPACING-ARROW_SPACING,0);
    }
  }
}
 
U0 DrawIt(CTask *,CDC *dc)
{
  I64 node_x=0,tree_x=0;
  ShowTree(dc,root,&node_x,&tree_x,20);
}

U0 TreeAdd(Node **_root,Node *tmpn)
{
  Node *root=*_root;
  if (!root)
    *_root=tmpn;
  else if (tmpn->n==root->n)
    Free(tmpn);
  else if (tmpn->n<root->n)
    TreeAdd(&root->left,tmpn);
  else
    TreeAdd(&root->right,tmpn);
}

U0 TreeNew()
{
  I64 i;
  Node *tmpn;
  for (i=0;i<N;i++) {
    tmpn=CAlloc(sizeof(Node));
    tmpn->n=RandU16%N;

    if (i==N-1)
      n1=tmpn->n;
    else if (i==N-2)
      n2=tmpn->n;

    TreeAdd(&root,tmpn);
    Sleep(50);
  }
}
 
U0 TreeCommonAncestorFind(Node *root)
{
  if (root && root->n!=n1 && root->n!=n2) {
    common_ancestor=root->n;
    if (n1<root->n && n2<root->n)
      TreeCommonAncestorFind(root->left);
    else if (n1>root->n && n2>root->n)
      TreeCommonAncestorFind(root->right);
  }
}
 
U0 TreeCommonAncestor()
{//Make tree and find common ancestor to n1 & n2.
  root=NULL;
  n1=n2=common_ancestor=0;

  SettingsPush; //See $LK,"SettingsPush",A="MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  DocClear;
  "Scroll with {CTRL-Left Grab}.\n";
  try {
    TreeNew;
    TreeCommonAncestorFind(root);
    PressAKey;
  } catch
    PutExcept;
  SettingsPop;
}

TreeCommonAncestor;
/*Be careful with recursive routines in TempleOS
because the stack does not grow and will overflow.

See $LK,"::/Demo/StkGrow.HC"$.
*/

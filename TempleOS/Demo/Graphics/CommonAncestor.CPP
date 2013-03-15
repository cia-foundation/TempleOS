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
 
U0 ShowTree(CDC *dc,Node *tempn,I64 *_node_x,I64 *_tree_x,I64 y)
{
  I64 node_x;
  if (tempn) {
    if (tempn->left) {
      ShowTree(dc,tempn->left,&node_x,_tree_x,y+Y_SPACING);
      dc->color=BLUE;
      GrArrow3(dc,*_tree_x,y,0,node_x+ARROW_SPACING,y+Y_SPACING-ARROW_SPACING,0);
    }
    if (tempn->n==n1 || tempn->n==n2) {
      if (tempn->n==common_ancestor)
	dc->color=YELLOW;
      else
	dc->color=RED;
    } else if (tempn->n==common_ancestor)
      dc->color=GREEN;
    else
      dc->color=BLUE;
 
    *_node_x=*_tree_x;
    GrPrintF(dc,*_node_x,y,"%d",tempn->n);
    *_tree_x+=X_SPACING;
 
    if (tempn->right) {
      ShowTree(dc,tempn->right,&node_x,_tree_x,y+Y_SPACING);
      dc->color=BLUE;
      GrArrow3(dc,*_node_x,y,0,node_x-ARROW_SPACING,y+Y_SPACING-ARROW_SPACING,0);
    }
  }
}
 
U0 DrawIt(CTask *,CDC *dc)
{
  I64 node_x=0,tree_x=0;
  ShowTree(dc,root,&node_x,&tree_x,20);
}

U0 TreeAdd(Node **_root,Node *tempn)
{
  Node *root=*_root;
  if (!root)
    *_root=tempn;
  else if (tempn->n==root->n)
    Free(tempn);
  else if (tempn->n<root->n)
    TreeAdd(&root->left,tempn);
  else
    TreeAdd(&root->right,tempn);
}

U0 TreeNew()
{
  I64 i;
  Node *tempn;
  for (i=0;i<N;i++) {
    tempn=CAlloc(sizeof(Node));
    tempn->n=RandU16%N;
 
    if (i==N-1)
      n1=tempn->n;
    else if (i==N-2)
      n2=tempn->n;
 
    TreeAdd(&root,tempn);
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
{ //Make tree and find common ancestor to n1 & n2.
  root=NULL;
  n1=n2=common_ancestor=0;
 
  SettingsPush; //See $LK,"SettingsPush","MN:SettingsPush"$
  Fs->draw_it=&DrawIt;
  DocClear;
  "Scroll with {CTRL-Left Grab}.\n";
  try {
    TreeNew;
    TreeCommonAncestorFind(root);
    PressAKey;
  } catch
    CatchAll;
  SettingsPop;
}

TreeCommonAncestor;
/*
Be careful with recursive routines in TempleOS
because the stack does not grow and will overflow.

See $LK-A,"::/TempleOS/Demo/StkGrow.CPP"$.
*/

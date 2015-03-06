/*标  题: fb家面试题讨论
发信站: BBS 未名空间站 (Thu Mar  5 20:03:48 2015, 美东)

给一个binary tree，返回一个circular doubly-linked list。


不好意思，忘说了，按inorder返回。*/

  TreeNode * convert(TreeNode * root) {
        if (! root) return NULL;

        TreeNode * head = NULL, * tail = NULL;

        stack<TreeNode *> s;
        TreeNode * n = root;
        while (1) {
            while (n) {
                s.push(n);
                n = n->left;
            }

            if (s.empty()) {
                if (head) {
                    head->left = tail;
                    tail->right = head;
                }
                break;
            }
            else {
                TreeNode * tmp = s.top();
                s.pop();
                n = tmp->right;
               
                cout << tmp->val << endl;
                append(tmp, head, tail);
            }
        }
       
        return head;
    }


    void append(TreeNode * t, TreeNode *& head, TreeNode *& tail) {
        if (head == NULL) {
            head = tail = t;
            t->left = t->right = NULL;
        }
        else {
            tail->right = t;
            t->left = tail;
            t->right = NULL;
            tail = t;
        }
    }

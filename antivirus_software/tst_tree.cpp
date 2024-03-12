/*#include <iostream>
using namespace std;

// Структура узла дерева
struct Node {
    //int key;
    char value;
    Node* left;
    Node* middle;
    Node* right;

    Node(char val) {
        //key=k;
        value = val;
        left = nullptr;
        middle = nullptr;
        right = nullptr;
    }
};

// Вставка
void insert(Node** root, char value) {
    if (*root == nullptr) {
        *root = new Node(value);
        return;
    }

    if (value < (*root)->value) {
        if ((*root)->left == nullptr)
            (*root)->left = new Node(value);
        else
            insert(&((*root)->left), value);
    }
    else if (value > (*root)->value) {
        if ((*root)->right == nullptr)
            (*root)->right = new Node(value);
        else
            insert(&((*root)->right), value);
    }
    else {
        if ((*root)->middle == nullptr)
            (*root)->middle = new Node(value);
        else
            insert(&((*root)->middle), value);
    }
}

// Поиск
bool search(Node* root, char value) {
    if (root == nullptr)
        return false;

    if (value < root->value)
        return search(root->left, value);
    else if (value > root->value)
        return search(root->right, value);
    else
        return true;
}

// Пример использования
int tst_main() {
    Node* root = new Node('E');
    insert(&root, 'C');
    insert(&root, 'G');
    insert(&root, 'A');
    insert(&root, 'D');

    cout << "Search results:" << endl;
    cout << "Value 'D': " << (search(root, 'D') ? "found" : "not found") << endl;
    cout << "Value 'G': " << (search(root, 'G') ? "found" : "not found") << endl;
    cout << "Value 'Z': " << (search(root, 'Z') ? "found" : "not found") << endl;

    return 0;
}*/
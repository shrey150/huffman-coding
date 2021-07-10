#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <functional>
#include <iomanip>

struct Node {

    char letter;
    int freq;

    Node* p;
    Node* left;
    Node* right;

    Node(char letter, int freq) 
    {
        std::cout << "(+) Node at " << this << std::endl;

        this->letter = letter;
        this->freq = freq;

        // leave branches empty
        p = nullptr;
        left = nullptr;
        right = nullptr;
    }

    ~Node()
    {
        std::cout << "(-) Node at " << this << std::endl;
    }
};

struct NodeComparator {

    bool operator() (Node* a, Node* b)
    {
        return a->freq > b->freq;
    }

};

auto gen_freqs(const std::string& str)
{
    // create map of letters and frequencies
    std::unordered_map<char,int> freqs;

    // loop through input string
    for (const char& letter: str)
    {
        // increment value corresponding to letter
        freqs[letter]++;
    }

    return freqs;
}

void print_node(const Node& n)
{
    std::cout << n.letter << " (code " << (int)n.letter << "): " << n.freq << std::endl; 
}

void print_tree_node(Node* n, bool is_leaf, int depth)
{
    char l = is_leaf ? '[' : '(';
    char r = is_leaf ? ']' : ')';

    std::string tab = std::string(depth, '\t');
    std::string info = std::to_string(n->freq);
    std::string end = is_leaf ? std::string("|") + (n->letter) : "";

    std::cout << tab << l << info << end << r << std::endl;
}

void print_tree(Node* n, int depth)
{
    // base case: stop traversing
    if (n == nullptr)
        return;

    // recursively print right branch
    print_tree(n->right, depth+1);

    // print current node
    print_tree_node(n, n->letter != 0, depth);

    // recursively print left branch
    print_tree(n->left, depth+1);
}

void print_tree(Node* root)
{
    std::cout << "\n\n";
    print_tree(root, 0);
    std::cout << "\n\n";
}

Node* gen_tree(std::string input, std::unordered_map<char, Node*>& lm)
{
    // create priority queue sorted in ascending order
    std::priority_queue<Node*, std::vector<Node*>, NodeComparator> pq;

    // calculate map of letters and frequencies
    auto freqs = gen_freqs(input);
    std::cout << "Nodes: " << freqs.size() << std::endl;

    for (const auto& f: freqs)
    {
        std::cout << "Adding node for '" << f.first << "'..." << std::endl;

        // add each letter-freq node to queue
        Node* n = new Node(f.first, f.second);
        pq.push(n);

        // map letter to node
        lm[f.first] = n;
    }

    std::cout << "Created priority queue of nodes." << std::endl;

    // loop until only the root node is left
    while (pq.size() > 1)
    {
        std::cout << "Removing two smallest nodes..." << std::endl;

        // remove two smallest nodes
        Node* a = pq.top();
        pq.pop();
        Node* b = pq.top();
        pq.pop();

        std::cout << "Connecting both nodes to parent..." << std::endl;

        // connect both nodes to parent node
        Node* c = new Node(0, a->freq+b->freq);
        c->left = a;
        c->right = b;
        a->p = b->p = c;
        
        std::cout << "Adding parent node back to queue..." << std::endl;

        // add parent node back to queue
        pq.push(c);
    }

    std::cout << "Tree complete." << std::endl;
    return pq.top();
}

auto encode(Node* n)
{
    std::vector<bool> bits;

    // traverse up the tree
    while (n->p != nullptr)
    {
        // if current node is left branch, prepend 0; else, prepend 1
        bits.insert(bits.begin(), (n == n->p->left ? 0 : 1));

        // switch current node to parent
        n = n->p;
    }

    return bits;
}

int main()
{
    std::string test = "I'd just like to interject for a moment. What you're referring to as Linux, is in fact, GNU/Linux, or as I've recently taken to calling it, GNU plus Linux. Linux is not an operating system unto itself, but rather another free component of a fully functioning GNU system made useful by the GNU corelibs, shell utilities and vital system components comprising a full OS as defined by POSIX. Many computer users run a modified version of the GNU system every day, without realizing it. Through a peculiar turn of events, the version of GNU which is widely used today is often called \"Linux\", and many of its users are not aware that it is basically the GNU system, developed by the GNU Project. There really is a Linux, and these people are using it, but it is just a part of the system they use. Linux is the kernel: the program in the system that allocates the machine's resources to the other programs that you run. The kernel is an essential part of an operating system, but useless by itself; it can only function in the context of a complete operating system. Linux is normally used in combination with the GNU operating system: the whole system is basically GNU with Linux added, or GNU/Linux. All the so-called \"Linux\" distributions are really distributions of GNU/Linux.";
    //std::string test = "this is an example of a huffman tree";

    std::unordered_map<char,Node*> lm;
    std::vector<bool> encoded;

    // generate Huffman tree
    Node* tree = gen_tree(test, lm);
    print_tree(tree);

    // encode string into bit stream
    for (const char& c: test)
    {
        // convert char to bits
        auto bits = encode(lm[c]);

        // append bits to encoded bit stream
        encoded.insert(encoded.end(), bits.begin(), bits.end());
    }

    // calculate size difference
    float pdiff = 100 - (float) encoded.size() / (test.length()*8) * 100;
    std::cout << std::setprecision(3);

    std::cout << "Uncompressed: " << test.length()*8 << std::endl;
    std::cout << "Compressed: " << encoded.size() << " (" << pdiff << "% smaller)" << std::endl;

    std::cout << "\nPress any key + enter to close.\t";
    std::string input;
    std::cin >> input;
    return 0;
}
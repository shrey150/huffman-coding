#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <iomanip>
#include <fstream>

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
    // CHANGE THIS FOR DIFFERENT INPUT TEXT
    std::string input_file = "examples/wikipedia.txt";

    // read input file as string
    std::ifstream ifs(input_file);
    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string input = buf.str();

    std::unordered_map<char,Node*> lm;
    std::vector<bool> encoded;

    // generate Huffman tree
    Node* tree = gen_tree(input, lm);
    print_tree(tree);

    // encode string into bit stream
    for (const char& c: input)
    {
        // convert char to bits
        auto bits = encode(lm[c]);

        // append bits to encoded bit stream
        encoded.insert(encoded.end(), bits.begin(), bits.end());
    }

    // DEBUG: print encoded bitstream
    // for (const auto& b: encoded)
    //     std::cout << b;

    // std::cout << std::endl;

    // calculate size difference
    float pdiff = 100 - (float) encoded.size() / (input.length()*8) * 100;
    std::cout << std::setprecision(3);

    std::cout << "Uncompressed: " << input.length()*8 << " bits" << std::endl;
    std::cout << "Compressed: " << encoded.size() << " bits (" << pdiff << "% smaller)" << std::endl;

    // dump bitstream to compressed file
    std::ofstream ofs(input_file.substr(input_file.find('/')+1) + ".huff", std::ofstream::binary);

    // temp byte to store bits
    char b = 0; 

    // loop through bitstream
    for (int i = 0; i < encoded.size(); ++i)
    {
        // every 8 bits, write byte
        if (i % 8 == 0 && i != 0)
        {
            // bulk write 8 bits 
            ofs.write(&b,1);
            
            // reset byte
            b = 0;
        }

        // append newest bit to byte
        b = (b << 1) + encoded[i];
    }

    // TODO: deal with "extra" 0 bits added to end of byte
    // (eg. 5 leftover bits will also have 3 meaningless 0 bits)

    // write any leftover bits in byte to file
    if (encoded.size()-1 % 8 != 0)
    {
        // shift bits until they line up w/ start of byte,
        // except don't shift if 8 bits are already in the byte
        // (eg. if 5 bits are used, shift left 3 times)
        if (encoded.size() % 8 != 0)
            b = b << (8 - encoded.size() % 8);

        // bulk write bits
        ofs.write(&b,1);
    }

    ofs.close();

    std::cout << "\nPress any key + enter to close.\t";
    std::string pause;
    std::cin >> pause;
    return 0;
}
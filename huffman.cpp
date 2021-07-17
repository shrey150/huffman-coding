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

void print_bits(std::vector<bool>& bits)
{
    for (const auto& b: bits)
        std::cout << b;

    std::cout << std::endl;
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

auto encode_msg(Node* n)
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

void encode_tree(Node* n, std::vector<bool> &bits)
{
    // if leaf node, append 1
    if (n->left == nullptr && n->right == nullptr)
    {
        bits.push_back(1);

        const char l = n->letter;

        // loop through bits of letter
        for (int i = 7; i >= 0; --i)
        {
            // append the current bit
            bits.push_back((l >> i) & 1);
        }
    }
    else
    {
        // for edge nodes, append 0
        bits.push_back(0);

        // recursively encode right then left branch
        encode_tree(n->right, bits);
        encode_tree(n->left, bits);
    }
}

Node* decode_tree(std::vector<bool> &bits, int& i)
{
    // decode next bit
    i++;

    // read leaf node
    if (bits[i] == 1)
    {
        char letter = 0;

        // set "end of char" index
        const int i_end = i + 8;

        // loop through bits of char
        for (++i; i <= i_end; ++i)
        {
            // add current bit to char
            letter = (letter << 1) + bits[i];            
        }

        // reset index to end of char
        --i;

        // create leaf node and return
        return new Node(letter, 0);
    }
    // read edge node
    else
    {
        // recursively decode right then left branch
        Node* right = decode_tree(bits, i);
        Node* left = decode_tree(bits, i);
        
        // create parent edge node and return
        Node* n = new Node(0, 0);
        n->left = left;
        n->right = right;
        return n;
    }
}

auto read_file(const std::string file_name)
{
    // read input file as string
    std::ifstream ifs(file_name, std::ios::binary);
    std::stringstream buf;
    buf << ifs.rdbuf();
    return buf.str();
}

void read_msg(std::string input_file)
{
    std::string input = read_file(input_file);
    std::vector<bool> bits;

    // loop through each char
    for (const char& c: input)
    {
        // loop through bits of char
        for (int i = 7; i >= 0; --i)
        {
            // append the current bit
            bits.push_back((c >> i) & 1);
        }
    }

    // DEBUG: print encoded bitstream
    //print_bits(bits);

    // needed to properly increment index
    int i = -1;

    // create tree from bitstream
    Node* tree = decode_tree(bits, i);
    print_tree(tree);

    // point to current node
    Node* n = tree;

    std::string msg;

    // loop through rest of bitstream
    for (++i; i < bits.size(); ++i)
    {
        // if we reached leaf node
        if (n->left == nullptr && n->right == nullptr)
        {
            // append leaf's letter to string
            msg.append(1, n->letter);

            // reset current node to parent
            n = tree;
        }
        
        // traverse down the tree
        n = bits[i] ? n->right : n->left;
    }

    std::cout << "Decoded message: " << msg << std::endl;
}

void write_msg(std::string input_file)
{
    std::string input = read_file(input_file);

    std::unordered_map<char,Node*> lm;
    std::vector<bool> bits;

    // generate Huffman tree
    Node* tree = gen_tree(input, lm);
    print_tree(tree);

    // encode tree into bit stream
    encode_tree(tree, bits);

    // encode string into bit stream
    for (const char& c: input)
    {
        // convert char to bits
        auto char_bits = encode_msg(lm[c]);

        // append bits to encoded bit stream
        bits.insert(bits.end(), char_bits.begin(), char_bits.end());
    }

    // DEBUG: print encoded bitstream
    //print_bits(bits);

    // calculate size difference
    float pdiff = 100 - (float) bits.size() / (input.length()*8) * 100;
    std::cout << std::setprecision(3);

    std::cout << "Uncompressed: " << input.length()*8 << " bits" << std::endl;
    std::cout << "Compressed: " << bits.size() << " bits (" << pdiff << "% smaller)" << std::endl;

    // dump bitstream to compressed file
    std::ofstream ofs(input_file.substr(input_file.find('/')+1) + ".huff", std::ofstream::binary);

    // temp byte to store bits
    char b = 0; 

    // loop through bitstream
    for (int i = 0; i < bits.size(); ++i)
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
        b = (b << 1) + bits[i];
    }

    // TODO: deal with "extra" 0 bits added to end of byte
    // (eg. 5 leftover bits will also have 3 meaningless 0 bits)

    // if bitstream does not evenly fit into bytes,
    // write any leftover bits in byte to file
    if (bits.size()-1 % 8 != 0)
    {
        // shift bits until they line up w/ start of byte (pad with 0s),
        // except don't shift if 8 bits are already in the byte
        // (eg. if 5 bits are used, shift left 3 times)
        if (bits.size() % 8 != 0)
            b = b << (8 - bits.size() % 8);

        // bulk write bits
        ofs.write(&b,1);
    }

    ofs.close();
}

int main()
{
    std::string input_file = "gnu";

    // encode + decode message
    write_msg("examples/" + input_file + ".txt");
    read_msg(input_file + ".txt.huff");

    std::cout << "\nPress any key + enter to close.\t";
    std::string pause;
    std::cin >> pause;
    return 0;
}
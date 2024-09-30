// #################################### From map.h
/*
class PathFinder 
{
public:
    enum class Dir
    {
        NORTH,
        SOUTH,
        WEST,
        EAST,
        NONE
    };

    struct Road
    {
        unsigned short startNodeId = 0; 
        unsigned short endNodeId = 0;
        unsigned int length = 0;
        PathFinder::Dir dir = PathFinder::Dir::NONE;
    };

    struct Node
    {
        unsigned short id = 0;
        int x, y;
        unsigned short nbAdjNodes;
        PathFinder::Road *path;

        Node(unsigned int id, int x, int y, int nbAdjNodes) : id(id), x(x), y(y), nbAdjNodes(nbAdjNodes) { path = new PathFinder::Road[nbAdjNodes]; }
        Node(const Node& node) : id(node.id), x(node.x), y(node.y), nbAdjNodes(node.nbAdjNodes) { path = new PathFinder::Road[nbAdjNodes]; for(int i = 0; i < nbAdjNodes; i++) path[i] = node.path[i]; }
        ~Node() { delete path; }
    };

    struct Path
    {
        unsigned short startNodeId = 0; 
        unsigned short endNodeId = 0;
        unsigned int length = 0;

        std::vector<PathFinder::Dir> dirList = {};

        //Path() {}
        //Path(const Path& path) : startNodeId(path.startNodeId), endNodeId(path.endNodeId), length(path.length) { dirList.insert(dirList.end(), path.dirList.begin(), path.dirList.end()); }

        Path operator+(const Path& rhs) const
        {
            Path newPath;
            newPath.startNodeId = startNodeId;
            newPath.endNodeId = rhs.endNodeId;
            newPath.length = this->length + rhs.length;
            newPath.dirList.insert(newPath.dirList.end(), dirList.begin(), dirList.end());
            newPath.dirList.insert(newPath.dirList.end(), rhs.dirList.begin(), rhs.dirList.end());

            return newPath;
        }

        Path operator-(const Path& rhs) const
        {
            Path newPath;
            newPath.startNodeId = startNodeId;
            newPath.endNodeId = rhs.endNodeId;
            newPath.length = this->length + rhs.length;

            std::vector<PathFinder::Dir> inverseDirList;

            for(auto dir : dirList)
            {
                //Inverse dir helper func
                switch (dir)
                {
                case PathFinder::Dir::NORTH:
                    inverseDirList.push_back(PathFinder::Dir::SOUTH);
                    break;
                case PathFinder::Dir::SOUTH:
                    inverseDirList.push_back(PathFinder::Dir::NORTH);
                    break;
                case PathFinder::Dir::EAST:
                    inverseDirList.push_back(PathFinder::Dir::WEST);
                    break;
                case PathFinder::Dir::WEST:
                    inverseDirList.push_back(PathFinder::Dir::EAST);
                    break;
                case PathFinder::Dir::NONE:
                    inverseDirList.push_back(PathFinder::Dir::NONE);
                    break;
                }
            }

            newPath.dirList.insert(newPath.dirList.end(), inverseDirList.rbegin(), inverseDirList.rend());
            newPath.dirList.insert(newPath.dirList.end(), rhs.dirList.begin(), rhs.dirList.end());

            return newPath;
        }

        Path& operator+=(const Path& rhs)
        {
            this->length += rhs.length;
            dirList.insert(dirList.end(), rhs.dirList.begin(), rhs.dirList.end());

            return *this;
        }

        //void operator=(const Path& rhs)
        //{
        //    this->startNodeId = rhs.startNodeId;
        //    this->endNodeId = rhs.endNodeId;
        //    this->length = rhs.length;
        //
        //    dirList.insert(dirList.end(), rhs.dirList.begin(), rhs.dirList.end());
        //}

        void operator=(const PathFinder::Road& rhs)
        {
            dirList.clear();

            this->startNodeId = rhs.startNodeId;
            this->endNodeId = rhs.endNodeId;
            this->length = rhs.length;
            
            dirList.insert(dirList.end(), this->length, rhs.dir);
        }

        Path operator-()
        {
            Path newPath;
            newPath.startNodeId = this->startNodeId;
            newPath.endNodeId = this->endNodeId;
            newPath.length = this->length;

            for(auto dir : dirList)
            {
                //Inverse dir helper func
                switch (dir)
                {
                case PathFinder::Dir::NORTH:
                    newPath.dirList.push_back(PathFinder::Dir::SOUTH);
                    break;
                case PathFinder::Dir::SOUTH:
                    newPath.dirList.push_back(PathFinder::Dir::NORTH);
                    break;
                case PathFinder::Dir::EAST:
                    newPath.dirList.push_back(PathFinder::Dir::WEST);
                    break;
                case PathFinder::Dir::WEST:
                    newPath.dirList.push_back(PathFinder::Dir::EAST);
                    break;
                case PathFinder::Dir::NONE:
                    newPath.dirList.push_back(PathFinder::Dir::NONE);
                    break;
                }
            }

            return newPath;
        }
    };

    PathFinder() {}
    ~PathFinder();

    void processMap(float **map, const int& width, const int& height);
    PathFinder::Path getPath(const constant::Vector2D& from, const constant::Vector2D& to);

private:
    std::vector<PathFinder::Node* > findCoreNodes(float **array, const int& width, const int& height);
    void generateAllPath(std::vector<PathFinder::Node* > *nodes);
    PathFinder::Path makePath(const unsigned int& startingNode, const unsigned int& endNode);

    float **map;
    int width = 0;
    int height = 0;

    unsigned int nbNodes = 0;
    std::vector<PathFinder::Node* > nodeList;
    PathFinder::Node ***nodeArray;
    PathFinder::Road ***roadArray;
    PathFinder::Path *currentPath;

    bool initialized = false;
};
*/
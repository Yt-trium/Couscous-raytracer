#include "kDTree.h"


// Constructors and destructor
template<class T>
kDTree<T>::kDTree()
{
    this->father = nullptr;
    this->firstChild = nullptr;
    this->secondCHild = nullptr;
    this->isLeaf = false;
    this->separationAxis = none;
    this->objectContainer = nullptr;
}

template<class T>
kDTree<T>::kDTree(kDTree *father, kDTreeObjectContainer<T> *obj)
{
    this->father = father;
    this->firstChild = nullptr;
    this->secondCHild = nullptr;
    this->isLeaf = true;
    this->separationAxis = none;
    this->objectContainer = obj;
}


template<class T>
kDTree<T>::kDTree(kDTree<T>* father, kDTree<T>* firstChild, kDTree<T>* secondCHild, kDTreeObjectContainer<T>* obj, enum enum_separationAxis sepAxis)
{
    this->father = father;
    this->firstChild = firstChild;
    this->secondCHild = secondCHild;
    this->isLeaf = false;
    this->objectContainer = obj;
    this->separationAxis = sepAxis;
}


template<class T>
kDTree<T>::~kDTree()
{
    if(!this->isLeaf)
    {
        if(this->firstChild != nullptr)
        {
            delete this->firstChild;
        }
        if(this->secondCHild != nullptr)
        {
            delete this->secondCHild;
        }
    }
}


// Getters and setters
template<class T>
kDTree<T>* kDTree<T>::getFather()
{
    return this->father;
}

template<class T>
void kDTree<T>::setFather(kDTree<T> *newFather)
{
    this->father = newFather;
}

template<class T>
kDTree<T>* kDTree<T>::getFirstChild()
{
    return this->firstChild;
}

template<class T>
void kDTree<T>::setFirstChild(kDTree<T> *newNode)
{
    this->firstChild = newNode;
}

template<class T>
kDTree<T>* kDTree<T>::getSecondChild()
{
    return this->secondCHild;
}

template<class T>
void kDTree<T>::setSecondChild(kDTree<T>* newNode)
{
    this->secondCHild = newNode;
}

template<class T>
bool kDTree<T>::isLeafNode() const
{
    return this->isLeaf;
}

template<class T>
void kDTree<T>::isLeafNode(bool newValue)
{
    this->isLeaf = newValue;
}

template<class T>
kDTreeObjectContainer<T>* kDTree<T>::getObjectContainer()
{
    return this->objectContainer;
}

template<class T>
void kDTree<T>::setObjectContainer(kDTreeObjectContainer<T>* objectContainer)
{
    this->objectContainer = objectContainer;
}

template<class T>
enum enum_separationAxis kDTree<T>::getSeparationAxis()
{
    return this->separationAxis;
}


// Auxiliary methods

template<class T>
kDTreeObjectContainer<T>* kDTree<T>::bissectricekDTreePoint(kDTreeObjectContainer<T>* obj1, kDTreeObjectContainer<T>* obj2)
{
    double x = (obj2->x() + obj1->x())/2;
    double y = (obj2->y() + obj1->y())/2;
    double z = (obj2->z() + obj1->z())/2;

    return new kDTreeObjectContainer<T>(x, y, z);
}

template<class T>
kDTree<T>* kDTree<T>::recursiveInitialization(std::vector<kDTreeObjectContainer<T> *> objectsList, enum enum_separationAxis divisionAxis)
{
    // lists of points on each side of the bissectrice
    std::vector<kDTreeObjectContainer<T>*>firstPart;
    std::vector<kDTreeObjectContainer<T>*>secondPart;
    kDTree<T>* fstChild;
    kDTree<T>* scdChild;
    kDTree<T>* answ;
    kDTreeObjectContainer<T>* bissectricePoint;

    // End of the recursion
    if(objectsList.size() == 1)
    {
        answ = new kDTree<T>(nullptr, *objectsList.begin());
        return answ;
    }

    switch(divisionAxis)
    {
        // X separation
        case separationIsOnX :
            // Sort the points on x coordinates
            std::sort(objectsList.begin(), objectsList.end(), classCompOnY<T>());
            break;
        // Y separation
        case separationIsOnY :
            // Sort the points on y coordinates
            std::sort(objectsList.begin(), objectsList.end(), classCompOnX<T>());
            break;
        // Z separation
        case separationIsOnZ :
            // Sort the points on z coordinates
            std::sort(objectsList.begin(), objectsList.end(), classCompOnZ<T>());
            break;
        default :
            std::cerr << "[WARNING]: in kDTree::recursiveInitialization - invalid axis enum" << std::endl;
            exit(1);
    }

    // Separate the vector in twice to get sets of points on each side of the bissectrice
    if(objectsList.size() != 2)
    {
        firstPart = std::vector<kDTreeObjectContainer<T>*>(objectsList.begin(), objectsList.begin() + (objectsList.size()/2)+1);
        secondPart = std::vector<kDTreeObjectContainer<T>*>((objectsList.begin() + (objectsList.size()/2))+1, objectsList.end());
    }
    else
    {
        firstPart.push_back(objectsList.front());
        secondPart.push_back(objectsList.back());
    }


    // Get bissectrice point
    //bissectricePoint = this->bissectricePoint(firstPart.back(), *secondPart.begin());
    bissectricePoint = firstPart.back();

    switch(divisionAxis)
    {
        case separationIsOnX :
            fstChild = this->recursiveInitialization(firstPart, separationIsOnY);
            scdChild = this->recursiveInitialization(secondPart, separationIsOnY);
            break;
        case separationIsOnY :
            fstChild = this->recursiveInitialization(firstPart, separationIsOnZ);
            scdChild = this->recursiveInitialization(secondPart, separationIsOnZ);
            break;
        case separationIsOnZ :
            fstChild = this->recursiveInitialization(firstPart, separationIsOnX);
            scdChild = this->recursiveInitialization(secondPart, separationIsOnX);
            break;
        default :
            std::cerr << "[WARNING]: in kDTree::recursiveInitialization - invalid axis enum" << std::endl;
            exit(1);
    }

    answ = new kDTree<T>(nullptr, fstChild, scdChild, bissectricePoint, divisionAxis);
    fstChild->setFather(answ);
    scdChild->setFather(answ);

    return answ;
}


template<class T>
float kDTree<T>::euclideanDistance(const kDTreeObjectContainer<T>* obj1, const kDTreeObjectContainer<T>* obj2) const
{
    return std::sqrt( std::pow(obj2->position.x - obj1->position.x, 2) + std::pow(obj2->position.y - obj1->position.y, 2) + std::pow(obj2->position.z - obj1->position.z, 2));
}


template<class T>
void kDTree<T>::recursiveSearch_kNearestNeighbors(const kDTreeObjectContainer<T>* refObject, kDTree<T> *node, const unsigned int neighborsNumbers)
{
    bool firstSubTreeBefore = true;
    float distance = 0.0f;
    kDTreePoint<T> currentPoint;

    if(node->objectContainer == refObject)
    {
        return;
    }

    distance = this->euclideanDistance(refObject, node->objectContainer);

    if(node->isLeafNode())
    {
        currentPoint.object =  node->objectContainer;
        currentPoint.distanceFromrefobject = distance;

        if(this->vectorKNN.size() == neighborsNumbers)
        {
            if(distance < this->vectorKNN.back().distanceFromrefobject)
            {
                this->vectorKNN.back() = currentPoint;
                std::sort(this->vectorKNN.begin(), this->vectorKNN.end(), kDTreePointComparisonClass<T>());
            }
        }
        else
        {
            this->vectorKNN.push_back(currentPoint);
            std::sort(this->vectorKNN.begin(), this->vectorKNN.end(), kDTreePointComparisonClass<T>());
        }
    }
    else if((this->vectorKNN.size() == 0) || (distance <= this->vectorKNN.back().distanceFromrefobject))
    {
        // Choose the first sub-tree to analyze

        switch(node->getSeparationAxis())
        {
            // If the bissectrice of the node is on x axis
            case separationIsOnX :
                if(refObject->position.y > node->objectContainer->position.y)
                {
                    firstSubTreeBefore = false;
                }
                break;
            // If the bissectrice of the node is on y axis
            case separationIsOnY :
                if(refObject->position.x > node->objectContainer->position.x)
                {
                    firstSubTreeBefore = false;
                }
                break;
            // If the bissectrice of the node is on z axis
            case separationIsOnZ :
                if(refObject->position.z > node->objectContainer->position.z)
                {
                    firstSubTreeBefore = false;
                }
                break;
            default :
                std::cerr << "[WARNING]: in kDTree, cannot determine axis of the node in function recursiveSearch_kNearestNeighbors" << std::endl;
                return;
        }

        if(firstSubTreeBefore)
        {
            this->recursiveSearch_kNearestNeighbors(refObject, node->getFirstChild(), neighborsNumbers);
            this->recursiveSearch_kNearestNeighbors(refObject, node->getSecondChild(), neighborsNumbers);
        }
        else
        {
            this->recursiveSearch_kNearestNeighbors(refObject, node->getSecondChild(), neighborsNumbers);
            this->recursiveSearch_kNearestNeighbors(refObject, node->getFirstChild(), neighborsNumbers);
        }
    }

    return;
}


// Methods

template<class T>
void kDTree<T>::init(std::vector<kDTreeObjectContainer<T>*> objectsList)
{
    // lists of points on each side of the bissectrice
    std::vector<kDTreeObjectContainer<T>*>firstPart;
    std::vector<kDTreeObjectContainer<T>*>secondPart;

    // Sort the points on x coordinates
    std::sort(objectsList.begin(), objectsList.end(), classCompOnX<T>());

    // Separate the vector in twice to get sets of points on each side of the bissectrice
    firstPart = std::vector<kDTreeObjectContainer<T>*>(objectsList.begin(), objectsList.begin() + (objectsList.size()/2));
    secondPart = std::vector<kDTreeObjectContainer<T>*>((objectsList.begin() + (objectsList.size()/2))+1, objectsList.end());

    // Get bissectrice point
    this->isLeaf = false;
    this->objectContainer = firstPart.back();

    // Compute children
    this->firstChild = recursiveInitialization(firstPart, separationIsOnY);
    this->secondCHild = recursiveInitialization(secondPart, separationIsOnY);

    this->separationAxis = separationIsOnX;
    this->firstChild->setFather(this);
    this->secondCHild->setFather(this);
}


template<class T>
std::vector<kDTreeObjectContainer<T>*> kDTree<T>::findKNN(const kDTreeObjectContainer<T> *refObject, const unsigned int neighborsNumbers)
{
    this->vectorKNN = std::vector<kDTreePoint<T>>();
    std::vector<kDTreeObjectContainer<T>*> answVector;

    this->recursiveSearch_kNearestNeighbors(refObject, this, neighborsNumbers);

    for(unsigned int i=0; i<this->vectorKNN.size(); i++)
    {
        answVector.push_back(this->vectorKNN[i].object);
    }

    return answVector;
}


// Debugging methods

template<class T>
std::string kDTree<T>::toString(int indent)
{
    std::string answ;
    for(int i=0; i<indent; i++)
    {
        answ += "\t";
    }

    if(this->isLeaf)
    {
        answ += "Leaf : (" + std::to_string(this->objectContainer->position.x) + ", " + std::to_string(this->objectContainer->position.y) + ", " + std::to_string(this->objectContainer->position.z) + "), sep Axis : " + std::to_string(this->separationAxis) + "\n";
    }
    else
    {
        answ += "Root : (" + std::to_string(this->objectContainer->position.x) + ", " + std::to_string(this->objectContainer->position.y) + ", " + std::to_string(this->objectContainer->position.z) + "), sep Axis : " + std::to_string(this->separationAxis) + "\n";
        if(this->firstChild != nullptr)
        {
            answ += this->firstChild->toString(++indent);
        }
        if(this->secondCHild != nullptr)
        {
            answ += this->secondCHild->toString(indent);
        }
    }

    return answ;
}











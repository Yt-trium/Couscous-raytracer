#ifndef __KDTREE_H
#define __KDTREE_H


#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <map>


struct Pos
{
public:
    float x;
    float y;
    float z;

    Pos()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    Pos(float x1, float y1, float z1)
    {
        x = x1;
        y = y1;
        z = z1;
    }
};

template<class Obj>
struct kDTreeObjectContainer
{
public:
    Obj object;
    Pos position;

    kDTreeObjectContainer(Obj object0, float x, float y, float z)
    {
        object = object0;
        position = Pos(x, y, z);
    }
};


enum enum_separationAxis {separationIsOnX, separationIsOnY, separationIsOnZ, none};

/**
 * @brief The classCompOnX struct is used to sort vertices in x axis
 */
template<class T>
struct classCompOnX
{
    bool operator() (kDTreeObjectContainer<T>* obj1, kDTreeObjectContainer<T>* obj2) const
    { return obj1->position.x<obj2->position.x; }
};

/**
 * @brief The classCompOnY struct is used to sort vertices in y axis
 */
template<class T>
struct classCompOnY
{
    bool operator() (kDTreeObjectContainer<T>* obj1, kDTreeObjectContainer<T>* obj2) const
    { return obj1->position.y<obj2->position.y; }
};

/**
 * @brief The classCompOnZ struct is used to sort vertices in z axis
 */
template<class T>
struct classCompOnZ
{
    bool operator() (kDTreeObjectContainer<T>* obj1, kDTreeObjectContainer<T>* obj2) const
    { return obj1->position.z<obj2->position.z; }
};


// modelisation of the base position of the holded object in the context of the kDTree
template<class T>
struct kDTreePoint
{
public:
    kDTreeObjectContainer<T>* object;
    float distanceFromrefobject;

    kDTreePoint() {}
};


template<class T>
struct kDTreePointComparisonClass
{
    bool operator() (const kDTreePoint<T>& p1, const kDTreePoint<T>& p2) const
    { return p1.distanceFromrefobject <= p2.distanceFromrefobject; }
};


/**
 * @brief The kDTree class represents the kDTree object with k = 2
 */
template<class T>
class kDTree
{
// Attributes
private:
    kDTree<T>* father;
    kDTree<T>* firstChild;
    kDTree<T>* secondCHild;
    bool isLeaf;
    enum enum_separationAxis separationAxis;
    /// kDTreePoint coordinates or kDTreePoint of the bissectrice
    kDTreeObjectContainer<T>* objectContainer;

// Attributes used for internal computations
private:
    /// Vector of kDTreePoint elements
    std::vector<kDTreePoint<T> > vectorKNN;


// Construtors and destructor
public:

    kDTree();

    /**
     * @brief kDTree constructor for a leaf node
     * @param father father node
     * @param pt kDTreeObjectContainer in the node
     */
    kDTree(kDTree<T> *father, kDTreeObjectContainer<T>* obj);

    /**
     * @brief kDTree constructor for non-leaf node
     * @param father father node
     * @param firstChild
     * @param secondCHild
     * @param pt kDTreePoint representing the bissectrice between the children
     * @param sepAxis axis of the bissectrice
     */
    kDTree(kDTree<T>* father, kDTree<T>* firstChild, kDTree<T>* secondCHild, kDTreeObjectContainer<T>* obj, enum_separationAxis sepAxis);

    ~kDTree();

// Getters and setters
public:


    /**
     * @brief getFather getter for the father of the node
     * @return
     */
    kDTree* getFather();


    /**
     * @brief setFather setter of the father of the node
     * @param newFather
     */
    void setFather(kDTree<T>* newFather);


    /**
     * @brief getFirstChild getter for the first child
     * @return
     */
    kDTree* getFirstChild();


    /**
     * @brief setFirstChild setter for the first child
     * @param newNode
     */
    void setFirstChild(kDTree<T>* newNode);


    /**
     * @brief getSecondChild getter for the second child
     * @return
     */
    kDTree* getSecondChild();


    /**
     * @brief setSecondChild setter for the second child
     * @param newNode
     */
    void setSecondChild(kDTree<T>* newNode);


    /**
     * @brief isLeafNode return true if the node is a leaf
     * @return
     */
    bool isLeafNode() const;


    /**
     * @brief isLeafNode set if the node is a leaf or not
     * @param newValue
     */
    void isLeafNode(bool newValue);


    /**
     * @brief object container getter for the object holded by the node
     * @return
     */
    kDTreeObjectContainer<T>* getObjectContainer();


    /**
     * @brief object container setter for the object that should be holded by the node
     * @param objectContainer
     */
    void setObjectContainer(kDTreeObjectContainer<T>* objectContainer);


    /**
     * @brief getSeparationAxis return the separation axis of the node
     * @return
     */
    enum enum_separationAxis getSeparationAxis();

// Auxiliary methods
private:


    /**
     * @brief recursiveInitialization used to build the tree
     * @param objectsList
     * @param divisionAxis
     * @return
     */
    kDTree* recursiveInitialization(std::vector<kDTreeObjectContainer<T> *> objectsList, enum_separationAxis divisionAxis);


    /**
     * @brief bissectrice kDTreePoint find the kDTreePoint between two given kDTreePoints
     * @param obj1
     * @param obj2
     * @return
     */
    kDTreeObjectContainer<T>* bissectricekDTreePoint(kDTreeObjectContainer<T>* obj1, kDTreeObjectContainer<T>* obj2);


    /**
     * @brief recursiveSearch_kNearestNeighbors recursive method to find the k Nearest Neighbors
     * @param node
     * @param neighborsNumbers
     */
    void recursiveSearch_kNearestNeighbors(const kDTreeObjectContainer<T>* refObject, kDTree* node, const unsigned int neighborsNumbers);


// Methods
public:


    /**
     * @brief euclideanDistance compute the euclidean distance
     * @param obj1
     * @param obj2
     * @return
     */
    float euclideanDistance(const kDTreeObjectContainer<T>* obj1, const kDTreeObjectContainer<T>* obj2) const;


    /**
     * @brief init Tree initialization
     * @param kDTreePoints
     */
    void init(std::vector<kDTreeObjectContainer<T>*> objectsList);



    /**
     * @brief findKNN find the k Nearest Neighbors of the given kDTreePoint
     * @param refobject
     * @param neighborsNumbers
     * @return
     */
    std::vector<kDTreeObjectContainer<T>*> findKNN(const kDTreeObjectContainer<T>* refObject, const unsigned int neighborsNumbers);


// Debugging methods
public:


    /**
     * @brief toString return the string version of the tree
     * @return
     */
    std::string toString(int indent);

};

template<class T>
inline std::ostream& operator<<(std::ostream& os, kDTree<T>& obj)
{
    os << obj.toString(0);
    return os;
}

template<class T>
inline std::ostream& operator<<(std::ostream& os, kDTree<T>* obj)
{
    os << obj->toString(0);
    return os;
}


#endif


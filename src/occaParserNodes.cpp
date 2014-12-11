#include "occaParserNodes.hpp"

namespace occa {
  namespace parserNS {
    //---[ Str Node ]-------------------------------
    strNode::strNode() :
      left(NULL),
      right(NULL),
      up(NULL),
      down(NULL),

      value(""),

      type(emptyType),
      depth(0),

      sideDepth(0) {}

    strNode::strNode(const std::string &value_) :
      left(NULL),
      right(NULL),
      up(NULL),
      down(NULL),

      value(value_),

      type(emptyType),
      depth(0),

      sideDepth(0) {}

    strNode::strNode(const strNode &n) :
      left(n.left),
      right(n.right),
      up(n.up),
      down(n.down),

      value(n.value),

      type(n.type),
      depth(n.depth),

      sideDepth(n.sideDepth) {}

    strNode& strNode::operator = (const strNode &n){
      left  = n.left;
      right = n.right;
      up    = n.up;
      down  = n.down;

      value = n.value;

      type  = n.type;
      depth = n.depth;

      sideDepth = n.sideDepth;

      return *this;
    }

    void strNode::swapWith(strNode *n){
      if(n == NULL)
        return;

      strNode *l1 = (left  == n) ? this : left;
      strNode *c1  = this;
      strNode *r1 = (right == n) ? this : right;

      strNode *l2 = (n->left  == this) ? n : n->left;
      strNode *c2 = right;
      strNode *r2 = (n->right == this) ? n : n->right;

      n->left  = l1;
      n->right = r1;

      left  = l2;
      right = r2;

      if(l1)
        l1->right = n;
      if(r1)
        r1->left  = n;

      if(l2)
        l2->right = this;
      if(r2)
        r2->left  = this;
    }

    void strNode::swapWithRight(){
      swapWith(right);
    }

    void strNode::swapWithLeft(){
      swapWith(left);
    }

    void strNode::moveLeftOf(strNode *n){
      if(n == NULL)
        return;

      if(left)
        left->right = right;
      if(right)
        right->left = left;

      left = n->left;

      if(n->left)
        left->right = this;

      right   = n;
      n->left = this;
    }

    void strNode::moveRightOf(strNode *n){
      if(n == NULL)
        return;

      if(left)
        left->right = right;
      if(right)
        right->left = left;

      right = n->right;

      if(n->right)
        right->left = this;

      left     = n;
      n->right = this;
    }

    strNode* strNode::clone() const {
      const strNode *nodePos = this;

      strNode *newNodeRoot = cloneNode();
      strNode *newNodePos  = newNodeRoot;

      while(nodePos->right){
        newNodePos->right       = nodePos->right->cloneNode();
        newNodePos->right->left = newNodePos;

        newNodePos = newNodePos->right;
        nodePos    = nodePos->right;
      }

      return newNodeRoot;
    }

    strNode* strNode::cloneTo(strNode *n) const {
      const strNode *nodePos = this;

      strNode *newNodeRoot = cloneNode();
      strNode *newNodePos  = newNodeRoot;

      while((nodePos->right) &&
            (nodePos != n)){
        newNodePos->right       = nodePos->right->cloneNode();
        newNodePos->right->left = newNodePos;

        newNodePos = newNodePos->right;
        nodePos    = nodePos->right;
      }

      return newNodeRoot;
    }

    strNode* strNode::cloneNode() const {
      strNode *newNode = new strNode();

      newNode->value = value;
      newNode->type  = type;

      newNode->up    = up;
      newNode->depth = depth;

      if(down)
        newNode->down = down->clone();

      return newNode;
    }

    strNode::operator std::string () const {
      return value;
    }

    strNode* strNode::pop(){
      if(left != NULL)
        left->right = right;

      if(right != NULL)
        right->left = left;

      return this;
    }

    strNode* strNode::push(strNode *node){
      strNode *rr = right;

      right = node;

      right->left  = this;
      right->right = rr;
      right->up    = up;

      if(rr)
        rr->left = right;

      return right;
    }

    strNode* strNode::push(const std::string &value_){
      strNode *newNode = new strNode(value_);

      newNode->type      = type;
      newNode->depth     = depth;
      newNode->sideDepth = sideDepth;

      return push(newNode);
    };

    strNode* strNode::pushDown(strNode *node){
      node->up = this;
      down     = node;
      return node;
    };

    strNode* strNode::pushDown(const std::string &value_){
      strNode *newNode = new strNode(value_);

      newNode->type  = type;
      newNode->depth = depth + 1;

      return pushDown(newNode);
    };

    node<strNode*> strNode::getStrNodesWith(const std::string &name_,
                                            const int type_){
      node<strNode*> nRootNode;
      node<strNode*> *nNodePos = nRootNode.push(new strNode());

      strNode *nodePos = this;

      while(nodePos){
        if((nodePos->type & everythingType) &&
           (nodePos->value == name_)){

          nNodePos->value = nodePos;
          nNodePos = nNodePos->push(new strNode());
        }

        if(down){
          node<strNode*> downRootNode = down->getStrNodesWith(name_, type_);

          if(downRootNode.value != NULL){
            node<strNode*> *lastDownNode = (node<strNode*>*) downRootNode.value;

            node<strNode*> *nnpRight = nNodePos;
            nNodePos = nNodePos->left;

            nNodePos->right       = downRootNode.right;
            nNodePos->right->left = nNodePos;

            nNodePos = lastDownNode;
          }
        }

        nodePos = nodePos->right;
      }

      nNodePos = nNodePos->left;

      if(nNodePos == &nRootNode)
        nRootNode.value = NULL;
      else
        nRootNode.value = (strNode*) nNodePos;

      delete nNodePos->right;
      nNodePos->right = NULL;

      return nRootNode;
    }

    void strNode::flatten(){
      strNode *nodePos = this;

      while(nodePos){
        if(down){
          strNode *oldRight = right;
          strNode *lastDown = lastNode(down);

          right      = down;
          down->left = this;
          down       = NULL;

          if(oldRight){
            lastDown->right = oldRight;
            oldRight->left  = lastDown;
          }

          nodePos = oldRight;
        }
        else
          nodePos = nodePos->right;
      }
    }

    bool strNode::freeLeft(){
      if((left != NULL) && (left != this)){
        if(left->down)
          free(left->down);

        strNode *l = left;

        left = l->left;

        if(left != NULL)
          left->right = this;

        delete l;

        return true;
      }

      return false;
    }

    bool strNode::freeRight(){
      if((right != NULL) && (right != this)){
        if(right->down)
          free(right->down);

        strNode *r = right;

        right = r->right;

        if(right != NULL)
          right->left = this;

        delete r;

        return true;
      }

      return false;
    }

    void strNode::print(const std::string &tab){
      strNode *nodePos = this;

      while(nodePos){
        if( !(nodePos->type & startSection) ){
          std::cout << tab << "[" << *nodePos << "] (" << getBits(nodePos->type) << ")\n";
        }
        else{
          const char startChar = nodePos->value[0];
          const char endChar   = segmentPair(startChar);

          const int startCharType = nodePos->type;
          const int endCharType   = ((startCharType & ~startSection) | endSection);

          printf("--------------------------------------------\n");
          std::cout << tab << "  " << "[" << startChar << "] (" << getBits(startCharType) << ")\n";

          if(nodePos->down)
            nodePos->down->print(tab + "  ");

          std::cout << tab << "  " << "[" << endChar << "] (" << getBits(endCharType) << ")\n";
          printf("--------------------------------------------\n");
        }

        nodePos = nodePos->right;
      }
    }

    std::string strNode::toString(const char spacing){
      const strNode *n = this;

      std::string ret;

      if(n){
        ret += n->value;
        n = n->right;
      }

      while(n){
        ret += spacing;
        ret += n->value;
        n = n->right;
      }

      return ret;
    }

    std::ostream& operator << (std::ostream &out, const strNode &n){
      out << n.value;
      return out;
    }

    void popAndGoRight(strNode *&node){
      strNode *left  = node->left;
      strNode *right = node->right;

      if(left != NULL)
        left->right = right;

      if(right != NULL)
        right->left = left;

      delete node;

      node = right;
    }

    void popAndGoLeft(strNode *&node){
      strNode *left  = node->left;
      strNode *right = node->right;

      if(left != NULL)
        left->right = right;

      if(right != NULL)
        right->left = left;

      delete node;

      node = left;
    }

    strNode* firstNode(strNode *node){
      if((node == NULL) ||
         (node->left == NULL))
        return node;

      strNode *end = node;

      while(end->left)
        end = end->left;

      return end;
    }

    strNode* lastNode(strNode *node){
      if((node == NULL) ||
         (node->right == NULL))
        return node;

      strNode *end = node;

      while(end->right)
        end = end->right;

      return end;
    }

    int length(strNode *node){
      int l = 0;

      while(node){
        ++l;
        node = node->right;
      }

      return l;
    }

    void free(strNode *node){
      if(node->down)
        free(node->down);

      while(node->freeRight())
        /* Do Nothing */;

      while(node->freeLeft())
        /* Do Nothing */;

      delete node;
    }
    //==============================================
  };
};

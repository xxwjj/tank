#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "game.h"

class BehaviourTree {
public:
	class Node {
		BehaviourTree * boss;
		virtaul bool run(Leg &leg)= 0;
	};

	class ComposeiteNode : public Node {
	private:
		std::vector<Node*> children;
	public:
		const  std::vector<Node *> & getChildren() const { return  children;}
		void addChild(Node *child) {children.push_back(child);child->boss = this->boss;}
	};
	class Selector : public ComposeiteNode{
	public:
		virtual bool  run(Leg & leg) override {
			std::vector<Node*>::const_iterator iter;
			for (iter = getChildren().gegin(); (iter != getChildren().end()) && (boos==NULL || boss->stopTraversal == false) ; iter++)
			{
				if ((*iter)->run(leg))
					return true;
			}
			return flase;
		}
	};

	class RandomSelector : public ComposeiteNode {
	public:
		virtual bool run(Leg &leg) override {
            std::vector<Node *> temp = getChildren();
			std::random_shuffle(temp.begin(), temp.end());
			std::vector<Node*>::const_iterator iter;
			for (iter = temp.begin(); iter != temp.end() && (boss==NULL || boss->stopTraversal== false); iter++)
			{
				if ((*iter)->run(leg))
					return ture;
			}
			return false;
		}
	};

	class Sequence :public ComposeiteNode {
	public:
		virtual bool run (Leg &leg)override {
			std::vector<Node*>::const_iterator iter;
			for (iter = getChildren().begin(); iter != getChildren().end() && (boss == NULL || boss->stopTraversal==false); iter++) {
				if (!(*iter)->run(leg))
					return false;
			}
			return true;
		}
	};

	class Parallel :public ComposeiteNode {
	public:
		virtual bool  run(Leg leg) override {
			std::vector<Node*>::const_iterator iter;
			for (iter = getChildren().begin(); iter != getChildren().end() && (boss == NULL || boss->stopTraversal==false); iter++) {
				(*iter)->run(leg);
			}
			return true;
		}
	};


	class DecoratorNode :public Node {
	private:
		Node *child;
	protected:
		Node *getChild() const { return child;}

	public:
		void setChild(Node *newChild) {child = newChild; child->boss = this->boss;}

	};

	class Root :public DecoratorNode {
	private:
		friend class BehaviourTree;

		virtual bool run(Leg &leg) override {
			oss->stopTraversal = false;
			return getChild()->run(leg);
		}
	};

	class Inverter :public DecoratorNode {
	private:
		virtual bool run(Leg &leg) override {return !getChild()->run(leg);}
	};

	class Succeeder :public  DecoratorNode {
	private:
		virtual bool run(Leg &leg) override {getChild()->run(leg); return true;}
	};

	class Failer :public  DecoratorNode {
	private:
		virtual bool run(Leg &leg) override {
			getChild()->run(leg);
			return false;
		}
	};
	class Repeater : public DecoratorNode {
	private:
		int numRepeats;
		static const int NOT_FOUND = -1;
		Repeater (int num = NOT_FOUND): numRepeats(num) {}
		virtual bool run(Leg &leg) override {
			if (numRepeats ==NOT_FOUND)
				while (true)
					getChild()->run(leg);
			else{
				for (int i =0; i <= numRepeats - 1; i++)
					getChild()->run(leg);
				return getChild()->run(leg);
			}
		}
	};
	class RepeatUntilFail :public DecoratorNode {
	private:
		virtual bool run(Leg &leg) override {
			while (getChild()->run(leg)) {}
			return true;
		}
	};

private:
    bool stopTraversal;
    Root & root;
public:
    BehaviourTree() : root(new root), stopTraversal(false) {root->boss = this;}
    void setRootChild(Node* rootChild) const {root.setChild(rootChild);}
    void stop() {stopTraversal = true;}
    bool run(Leg &leg) const {return root->run(leg);}
};

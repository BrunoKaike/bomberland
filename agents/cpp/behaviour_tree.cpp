#include <iostream>
#include <list>
#include "player_states.cpp"

using namespace std;

string ação;

class Node {  
	public:
		virtual bool run() = 0;
};

class CompositeNode : public Node {  

	private:
		std::list<Node*> children;
	public:
		const std::list<Node*>& getChildren() const {return children;}
		void addChild (Node* child) {children.emplace_back(child);}
};

class Selector : public CompositeNode {
	public:
		virtual bool run() override {
			for (Node* child : getChildren()) {  
				if (child->run())
					return true;
			}
			return false;
		}
};

class Sequence : public CompositeNode {
	public:
		virtual bool run() override {
			for (Node* child : getChildren()) {  
				if (!child->run())  
					return false;
			}
			return true; 
		}
};

class PosicionarBomba : public Node {  
	private:
		bool input;
	public:
		PosicionarBomba (bool input) : input(input) {}
		virtual bool run() override {
			if(input == true){

        ação = "posicionarBomba";
        
      } 

      return input;
		}
};


class ExplodirBomba : public Node {  
	private:
		bool input;
	public:
		ExplodirBomba (bool input) : input(input) {}
		virtual bool run() override {
			if(input == true){

        ação = "explodirBomba";
        
      } 

      return input;
		}
};


class FugirDePerigo : public Node {  
	private:
		bool input;
	public:
		FugirDePerigo (bool input) : input(input) {}
		virtual bool run() override {
			if(input == true){

        ação = "fugir";
        
      } else {

        ação = "quebrarCaixa";
        
      }

      return input;
		}
};

class PegarPowerUp : public Node {  
	private:
		bool input;
	public:
		PegarPowerUp (bool input) : input(input) {}
		virtual bool run() override {
			if(input == true){

        ação = "pegarPowerUp";
        
      }

      return input;
		}
};

class PerseguirInimigo : public Node {  
	private:
		bool input;
	public:
		PerseguirInimigo (bool input) : input(input) {}
		virtual bool run() override {
			if(input == true){

        ação = "perseguirInimigo";
        
      }

      return input;
		}
};

string behaviourTree(Estado estadoGame) {

  Selector* main = new Selector;
  Selector* selectorGeral = new Selector;
  PosicionarBomba* posicionarBomba = new PosicionarBomba(estadoGame.estaVizinho && estadoGame.pegouPowerUpGelo);
  ExplodirBomba* explodirBomba = new ExplodirBomba(estadoGame.inimigoEmPerigo && !estadoGame.estaEmPerigo);
  PegarPowerUp* pegarPowerUp = new PegarPowerUp(estadoGame.powerUpNoMapa);
  PerseguirInimigo* perseguirInimigo = new PerseguirInimigo(estadoGame.pegouPowerUpGelo && !estadoGame.estaEmPerigo);
  FugirDePerigo* fugirDePerigo = new FugirDePerigo(estadoGame.estaEmPerigo);

  main->addChild (selectorGeral);
	
	selectorGeral->addChild (fugirDePerigo);
	selectorGeral->addChild (pegarPowerUp);
	selectorGeral->addChild (perseguirInimigo);
  selectorGeral->addChild (posicionarBomba);
  selectorGeral->addChild (explodirBomba);
  
	while (!main->run()) {}

	cout << "Ação determinada pera árvore de comportamento: " << ação << endl;
  
	return ação;
}
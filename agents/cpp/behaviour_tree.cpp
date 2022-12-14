#include <iostream>
#include <list>
#include "player_states.cpp"

using namespace std;

string acao;

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

        acao = "posicionarBomba";
        
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

        acao = "explodirBomba";
        
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

        acao = "fugir";
        
      } else {

        acao = "quebrarCaixa";
        
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

        acao = "pegarPowerUp";
        
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

        acao = "perseguirInimigo";
        
      }

      return input;
		}
};

class Default : public Node {  
	private:
		bool input;
	public:
		Default (bool input) : input(input) {}
		virtual bool run() override {
			if(input == true){

        acao = "quebrarCaixa";
        
      }

      return input;
		}
};

string behaviourTree(Estado estadoGame) {

  Selector* main = new Selector;
  Selector* selectorGeral = new Selector;
  PosicionarBomba* posicionarBomba = new PosicionarBomba(!estadoGame.estaEmPerigo &&estadoGame.estaVizinho && estadoGame.pegouPowerUpGelo);
  ExplodirBomba* explodirBomba = new ExplodirBomba(!estadoGame.estaEmPerigo && estadoGame.inimigoEmPerigo && !estadoGame.estaVizinho);
  PegarPowerUp* pegarPowerUp = new PegarPowerUp(!estadoGame.estaEmPerigo && estadoGame.powerUpNoMapa && estadoGame.pegouPowerUpGelo);
  PerseguirInimigo* perseguirInimigo = new PerseguirInimigo(!estadoGame.estaEmPerigo && !estadoGame.estaVizinho && estadoGame.pegouPowerUpGelo);
  FugirDePerigo* fugirDePerigo = new FugirDePerigo(estadoGame.estaEmPerigo);
  Default* acaoDefault = new Default(true);

  main->addChild (selectorGeral);
	
	selectorGeral->addChild (fugirDePerigo);
	selectorGeral->addChild (pegarPowerUp);
	selectorGeral->addChild (perseguirInimigo);
  	selectorGeral->addChild (posicionarBomba);
  	selectorGeral->addChild (explodirBomba);
  	selectorGeral->addChild (acaoDefault);
  
	while (!main->run()) {}

	std::cout << "Acao determinada pera árvore de comportamento: " << acao << std::endl;
  
	return acao;
}

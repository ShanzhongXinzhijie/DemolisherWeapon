#pragma once
class IRander
{
public:
	virtual ~IRander() {};

	virtual void Render()=0;
	virtual void PostRender() {};
};

class RanderManager {
public:
	
	void Render() {
		for (auto& r : m_renderList) {
			r.second->Render();
		}
		for (auto& r : m_renderList) {
			r.second->PostRender();
		}
	};

	void AddRender(const int priority, IRander* render) {
		m_renderList.emplace(priority, render);
	}
	void DeleteRender(const int priority){
		m_renderList.erase(priority);
	}
	void DeleteRender(IRander* render) {
		for (auto itr = m_renderList.begin();itr != m_renderList.end();++itr) {
			if(itr->second == render){
				m_renderList.erase(itr);
				break;
			}
		}
	}

private:
	std::multimap<int, IRander*> m_renderList;
};

//•`‰æ‡ŠÖŒW
static const int DRAW_PRIORITY_MAX = 5;
static const int DRAW_PRIORITY_DEFAULT = 2;
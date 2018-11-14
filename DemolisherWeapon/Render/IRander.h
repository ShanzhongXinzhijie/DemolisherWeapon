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

private:
	std::multimap<int, IRander*> m_renderList;
};
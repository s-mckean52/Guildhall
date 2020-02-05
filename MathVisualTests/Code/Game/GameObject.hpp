#pragma once

class Rigidbody2D;

class GameObject
{
public:
	GameObject();
	~GameObject();

	void Draw() const;

public:
	Rigidbody2D* m_rigidbody = nullptr;
};
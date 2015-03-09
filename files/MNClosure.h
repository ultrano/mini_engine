#ifndef _H_MNClosure
#define _H_MNClosure

#include "MNCollectable.h"
#include "MNObject.h"
class MNFiber;

struct UpValue : public MNMemory
{
	virtual ~UpValue() {};
	virtual bool isOpened() = 0;
	virtual const MNObject& get() = 0;
	virtual void	set(const MNObject& v) = 0;
};

struct Opened : UpValue
{
	MNFiber* fiber;
	tuint	 index;
	Opened();
	Opened(MNFiber* f, tuint i);
	~Opened();
	bool	isOpened() override;
	const MNObject& get() override;
	void	set(const MNObject& v) override;;
};

struct Closed : UpValue
{
	MNObject val;
	Closed();
	Closed(const MNObject& v);
	~Closed();
	bool	isOpened() override;
	const MNObject& get() override;
	void	set(const MNObject& v) override;;
};

class UpLink : public MNMemory
{
public:

	UpValue* link;
	tuint    nref;

	UpLink();
	UpLink(const MNObject& v);
	UpLink(MNFiber* fiber, tuint index);
	~UpLink();
	const MNObject& get();
	void	set(const MNObject& v);
	void	close();
	void    inc();
	void    dec();
};

class MNClosure : public MNCollectable
{
	MN_RTTI(MNClosure, MNCollectable);

public:

	MNClosure();
	MNClosure(MNObject func);
	~MNClosure();

	UpLink*	getLink(tuint idx);
	void	addLink(UpLink* link);

	const MNObject&	getUpval(tuint idx);
	void	setUpval(tuint idx, const MNObject& v);

	bool    isNative() const;

	void bindThis(MNObject _this);
	const MNObject& getThis();

	const MNObject& getFunc() const;
	void setFunc(const MNObject& func);

protected:

	virtual void travelMark();

private:

	tarray<UpLink*> m_uplinks;
	MNObject m_func;
	MNObject m_this;
};

#endif
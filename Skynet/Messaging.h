#pragma once

#include <vector>
#include <functional>
#include <tuple>

template <typename T>
class MessageReporterBase;

template <typename T>
class MessageListenerBase
{
public:
	MessageListenerBase( MessageReporterBase<T> &handler )
		: m_handler( &handler )
	{
		m_handler->registerForMessage( this );
	}

	virtual ~MessageListenerBase()
	{
		m_handler->unRegisterForMessage( this );
	}

	virtual void notify( const T & message ) = 0;

	MessageListenerBase( const MessageListenerBase & ) = delete;
	MessageListenerBase & operator=( const MessageListenerBase & ) = delete;

private:
	MessageReporterBase<T> *m_handler;
};

template <typename T>
class MessageListenerFunction : MessageListenerBase<T>
{
public:
	MessageListenerFunction( MessageReporterBase<T> &handler, std::function<void( const T & )> function )
		: MessageListenerBase<T>( handler )
		, m_function( std::move( function ) )
	{
	}

	void notify( const T & message ) override
	{
		m_function( message );
	}

private:
	std::function<void( const T & )> m_function;
};

template <typename T>
class MessageReporterBase
{
public:
	MessageReporterBase() = default;
	MessageReporterBase( const MessageReporterBase & ) = delete;
	MessageReporterBase & operator=( const MessageReporterBase & ) = delete;

	void basePostMessage( const T & message )
	{
		for( auto & listener : m_listeners )
			listener->notify( message );
	}

private:
	friend class MessageListenerBase<T>;

	std::vector<MessageListenerBase<T>*> m_listeners;

	void registerForMessage( MessageListenerBase<T>* listener )
	{
		m_listeners.emplace_back( listener );
	}

	void unRegisterForMessage( MessageListenerBase<T>* listener )
	{
		auto it = std::find( m_listeners.begin(), m_listeners.end(), listener );
		if( it != m_listeners.end() )
		{
			m_listeners.erase( it );
		}
	}
};

template <typename... MESSSAGES>
class MessageReporter : public MessageReporterBase<MESSSAGES>...
{
public:
	template <typename T>
	void postMessage( const T & message )
	{
		static_assert(std::is_base_of<MessageReporterBase<T>, MessageReporter>::value, "Reporter cannot handle that message type");
		static_cast<MessageReporterBase<T>*>(this)->basePostMessage( message );
	}

	template <typename T>
	void postMessage()
	{
		static_assert(std::is_base_of<MessageReporterBase<T>, MessageReporter>::value, "Reporter cannot handle that message type");
		static_cast<MessageReporterBase<T>*>(this)->basePostMessage( T() );
	}
};

template <typename... MESSSAGES>
class MessageListener : public MessageListenerBase<MESSSAGES>...
{
public:
	template <typename T>
	MessageListener( T &handler )
		: MessageListenerBase<MESSSAGES>(handler)...
	{
	}
};
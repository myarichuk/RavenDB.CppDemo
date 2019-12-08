// RavenDB.CppDemo.cpp : Defines the entry point for the application.
//

#include "RavenDB.CppDemo.h"
#include <algorithm>
using namespace std;
using namespace ravendb::client::serverwide::operations;

struct contact_info
{
	string country;
	string city;
	string address;

	vector<string> emails;
};

inline void to_json(nlohmann::json& j, const contact_info& c)
{
	using ravendb::client::impl::utils::json_utils::set_val_to_json;
	set_val_to_json(j, "city", c.city);
	set_val_to_json(j, "country", c.country);
	set_val_to_json(j, "address", c.address);
	set_val_to_json(j, "emails", c.emails);
}

inline void from_json(const nlohmann::json& j, contact_info& c)
{
	using ravendb::client::impl::utils::json_utils::get_val_from_json;
	
	get_val_from_json(j, "city", c.city);
	get_val_from_json(j, "country", c.country);
	get_val_from_json(j, "address", c.address);
	get_val_from_json(j, "emails", c.emails);
}

struct user
{
	string id;
	string name;
	int age{};
	contact_info contact_info;
};

inline void to_json(nlohmann::json& j, const user& u)
{
	using ravendb::client::impl::utils::json_utils::set_val_to_json;
	set_val_to_json(j, "name", u.name);
	set_val_to_json(j, "age", u.age);
	set_val_to_json(j, "contact_info", u.contact_info);
}

inline void from_json(const nlohmann::json& j, user& u)
{
	using ravendb::client::impl::utils::json_utils::get_val_from_json;
	get_val_from_json(j, "name", u.name);
	get_val_from_json(j, "age", u.age);
	get_val_from_json(j, "contact_info", u.contact_info);
}



int main()
{
	cout << "Hello RavenDB!" << endl;

	REGISTER_ID_PROPERTY_FOR(user, id);

	/*
	Initialize an object that represents a document store.
	It will serve as an "access point" of all RavenDB functionality.
	Note that the creation of DocumentStore has an overhead, so ideally it
	would be a singleton
	*/
	auto store = ravendb::client::documents::DocumentStore::create();

	/*
	Now we configure the DocumentStore
	Set at least one url of a RavenDB cluster and the client will fetch topology of the cluster after the first request.
	Ideally there should be more than one urls here - in this way the initialization would be more resilient since the 
	client API will try to connect for multiple nodes
	*/
	store->set_urls({ "http://live-test.ravendb.net" }); //assuming there is unsecure RavenDB instance at this url

	/*
	Set the default database to connect to. 
	It is possible to specify other databases, so this setting is mostly for convenience
	*/
	store->set_database("TestDB");

	//after this, no more configuration of the DocumentStore is possible
	store->initialize();

	//fetch all existing database names
	//notice 'store' being an entry point for an operation
	const auto databaseNames = *store->maintenance()->server()->send(GetDatabaseNamesOperation(0, INT_MAX));

	//create TestDB if not exists
	if(find(databaseNames.begin(), databaseNames.end(), "TestDB") == databaseNames.end())
	{
		auto dr = ravendb::client::serverwide::DatabaseRecord();
		dr.database_name = "TestDB";

		//executing this creates a database, if it already exists, throws
		store->maintenance()->server()->send(CreateDatabaseOperation(dr));
	}

{
	//opening a session encapsulates transaction
    auto session = store->open_session("TestDB"); 
    auto user_record = std::make_shared<user>();
    user_record->name = "John Dow";
    user_record->age = 35;
	user_record->contact_info = 
	contact_info
		{
			"Dreamland",
			"Fairy City",
			"Sesame str. 123/456",
			{"foo@bar.com","non_existing@mail_domain.com","bar@foo.com"}
		};
    session.store(user_record);

    auto user_record2 = std::make_shared<user>();
    user_record2->name = "Jane Dow";
    user_record2->age = 24;
	user_record2->contact_info = 
	contact_info
		{
			"Jotunheimr",
			"Jotunfjord",
			"Frost str. 45/67",
			{"abc@foobar.com", "jane.dow@foobar.com"}
		};
    session.store(user_record2);
	
	session.save_changes(); //tx commit, we talk with server here
}

	//now fetch some data from server
	{
		auto session = store->open_session();

		//fetch record with id = 'users/1-A'
		const auto john = *session.load<user>("users/1-A");

		cout<< john.name << "'s age is " << john.age << endl;

		//execute query and fetch all users that match filtering criteria with sorting
		const std::vector<std::shared_ptr<user>> usersNamedJane = session.query<user>()
		                                                                 ->where_starts_with("name", "Jane")
		                                                                 ->order_by_descending("name")
		                                                                 ->to_list();		
		//should be always one user
		cout << "found " << usersNamedJane.size() << " user(s) with name 'Jane'" << endl;
		cout<< usersNamedJane[0]->name << "'s age is " << usersNamedJane[0]->age << endl;
		for(const auto& email : usersNamedJane[0]->contact_info.emails)
			cout << "email:" << email << endl;
	}

	//delete the database on the server
	store->maintenance()->server()->send(DeleteDatabasesOperation("TestDB", true));

 	return 0;
}

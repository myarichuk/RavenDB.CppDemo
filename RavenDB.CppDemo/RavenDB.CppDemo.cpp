// RavenDB.CppDemo.cpp : Defines the entry point for the application.
//

#include "RavenDB.CppDemo.h"
#include <algorithm>
using namespace std;
using namespace ravendb::client::serverwide::operations;

struct User
{
	string id;
	string name;
	int age{};

	friend bool operator==(const User& lhs, const User& rhs)
	{
		return lhs.name == rhs.name
			&& lhs.id == rhs.id
			&& lhs.age == rhs.age;
	}
};

	inline void to_json(nlohmann::json& j, const User& u)
	{
		using ravendb::client::impl::utils::json_utils::set_val_to_json;

		set_val_to_json(j, "name", u.name);
		set_val_to_json(j, "age", u.age);
	}

	inline void from_json(const nlohmann::json& j, User& u)
	{
		using ravendb::client::impl::utils::json_utils::get_val_from_json;

		get_val_from_json(j, "name", u.name);
		get_val_from_json(j, "age", u.age);
	}



int main()
{
	cout << "Hello RavenDB!" << endl;

	REGISTER_ID_PROPERTY_FOR(User, id);

	auto store = ravendb::client::documents::DocumentStore::create();
	store->set_urls({ "http://localhost:8080" });
	store->set_database("TestDB");
	store->initialize();

	//fetch all existing database names
	
	const auto databaseNames = *store->maintenance()->server()->send(GetDatabaseNamesOperation(0, INT_MAX));

	//create TestDB if not exists
	if(find(databaseNames.begin(), databaseNames.end(), "TestDB") == databaseNames.end())
	{
		auto dr = ravendb::client::serverwide::DatabaseRecord();
		dr.database_name = "TestDB";
		store->maintenance()->server()->send(CreateDatabaseOperation(dr));
	}

    { //first store some data

		//opening a session encapsulates transaction
        auto session = store->open_session("TestDB"); 
        auto user = std::make_shared<User>();
        user->name = "John Dow";
        user->age = 35;
        session.store(user);

        auto user2 = std::make_shared<User>();
        user2->name = "Jane Dow";
        user2->age = 24;
        session.store(user2);

		session.save_changes(); //tx commit
    }

	//now fetch some data from server
	{
		auto session = store->open_session();
		const auto john = *session.load<User>("users/1-A");

		cout<< john.name << "'s age is " << john.age << endl;

		//execute query and fetch all users that match filtering criteria
		const auto usersNamedJane = session.query<User>()
											->where_starts_with("name","Jane")
											->to_list();

		//should be always one user
		cout << "found " << usersNamedJane.size() << " user(s) with name 'Jane'" << endl;
		cout<< usersNamedJane[0]->name << "'s age is " << usersNamedJane[0]->age << endl;		
	}

	//delete the database on the server
	store->maintenance()->server()->send(DeleteDatabasesOperation("TestDB", true));

 	return 0;
}

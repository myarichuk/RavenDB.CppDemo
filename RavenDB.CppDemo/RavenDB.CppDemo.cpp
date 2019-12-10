// RavenDB.CppDemo.cpp : Defines the entry point for the application.
//

#include "RavenDB.CppDemo.h"
#include <algorithm>
#include <array>
using namespace std;
using namespace ravendb::client::serverwide::operations;


struct user
{
	string id;
	string name;
	int age{};
	vector<string> emails;
};

inline void to_json(nlohmann::json& j, const user& u)
{
	using ravendb::client::impl::utils::json_utils::set_val_to_json;
	set_val_to_json(j, "name", u.name);
	set_val_to_json(j, "age", u.age);
	set_val_to_json(j, "emails", u.emails);
}

inline void from_json(const nlohmann::json& j, user& u)
{
	using ravendb::client::impl::utils::json_utils::get_val_from_json;
	get_val_from_json(j, "name", u.name);
	get_val_from_json(j, "age", u.age);
	get_val_from_json(j, "emails", u.emails);
}

struct email_count_result
{
	string email;
	int count{};
};

inline void to_json(nlohmann::json& j, const email_count_result& u)
{
	using ravendb::client::impl::utils::json_utils::set_val_to_json;
	set_val_to_json(j, "email", u.email);
	set_val_to_json(j, "count", u.count);
}

inline void from_json(const nlohmann::json& j, email_count_result& u)
{
	using ravendb::client::impl::utils::json_utils::get_val_from_json;
	get_val_from_json(j, "email", u.email);
	get_val_from_json(j, "count", u.count);
}

int main()
{
	//let RavenDB client know what is the 'id' of the document class
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

	//if TestDB exists, delete it to have "fresh" database for demo
	if(find(databaseNames.begin(), databaseNames.end(), "TestDB") != databaseNames.end())
	{
		//delete the database on the server
		store->maintenance()->server()->send(DeleteDatabasesOperation("TestDB", true));
		cout << "deleted database 'TestDB'" <<endl;
	}

	auto dr = ravendb::client::serverwide::DatabaseRecord();
	dr.database_name = "TestDB";
	//executing this creates a database, if it already exists, throws
	store->maintenance()->server()->send(CreateDatabaseOperation(dr));
	cout << "created database 'TestDB'" <<endl;
	
{
	//opening a session encapsulates transaction
    auto session = store->open_session(); 
    auto user_record = std::make_shared<user>();
    user_record->name = "John Doe";
    user_record->age = 35;
	user_record->emails = {"john.doe@example.com","abc@example.com","bar@example.com"};
    session.store(user_record); //this doesn't do roundtrip to the server

    auto user_record2 = std::make_shared<user>();
    user_record2->name = "Jane Doe";
    user_record2->age = 24;
	user_record2->emails = {"abc@example.com", "jane.doe@example.com"};
    session.store(user_record2); //this doesn't do roundtrip to the server as well

	auto user_record3 = std::make_shared<user>();
    user_record3->name = "Jack Foobar";
    user_record3->age = 28;
	user_record3->emails = {"jack.foobar@example.com", "abc@example.com", "bar@example.com"};
    session.store(user_record3); 
	
	session.save_changes(); //tx commit, we talk with server here
	cout << "saved some data to database..." <<endl;

}

	//now fetch some data from server
	{
		auto session = store->open_session();

		//fetch record with id = 'users/1-A'

		const auto john = *session.load<user>("users/1-A");

		cout << "============" <<endl;
		cout << "load by id results" <<endl;
		cout<< john.name << "'s age is " << john.age << endl;

		cout << "============" <<endl;
		cout << "raw query #1 results" <<endl;

		//full-text search on 'name' field
		const std::vector<std::shared_ptr<user>> rawUsersNamedJane = 
			session.advanced().raw_query<user>("from users where search(name, $name_to_search)")
							  ->add_parameter("name_to_search","john")
							  ->to_list();
		for(const auto& user : rawUsersNamedJane)
			cout << "name:" << user->name << endl;

		cout << "============" <<endl;
		cout << "raw query #2 results" <<endl;
		
		
		const char *aggregation_query = 
		   "from users "
		   "group by emails[] "
		   "order by count() desc "
		   "select key() as email, count() as count";

		const auto rawCountEmailsPerUser = 
			session.advanced().raw_query<email_count_result>
				(aggregation_query)->to_list();

		for(const auto& result : rawCountEmailsPerUser)
			cout << "email:" << result->email << ", count: " << result->count << endl;

		//full-text search on 'name' field
		const vector<shared_ptr<user>> rawCountUsersPerEmail = session.advanced().raw_query<user>("from users where search(name, 'john')")->to_list();
		for(const auto& user : rawUsersNamedJane)
			cout << "name:" << user->name << endl;

		cout << "============" <<endl;
		cout << "raw query #3 results" <<endl;

		std::string complex_query = 
		   "from users "
		   "where emails[] in ( $email_list ) or"
		   "(age > 25 and endsWith(name, 'Doe'))";

		const vector<shared_ptr<user>> complexQueryResult = 
			session.advanced().raw_query<user>(complex_query)
			       ->add_parameter("email_list",
			           nlohmann::json::array(
						   {"john.doe@example.com","jane.doe@example.com" }))
		           ->to_list();

		for(const auto& user : complexQueryResult)
			cout << "name:" << user->name << endl;

		cout << "============" <<endl;
		//execute query and fetch all users that match filtering criteria with sorting
		const std::vector<std::shared_ptr<user>> usersNamedJane = session.query<user>()
		                                                                 ->where_starts_with("name", "Jane")
		                                                                 ->order_by_descending("name")
		                                                                 ->to_list();

		cout << "============" << endl;
		cout << "query #1 results" << endl;
		cout << "found " << usersNamedJane.size() << " user(s) with name 'Jane'" << endl;
		cout<< usersNamedJane[0]->name << "'s age is " << usersNamedJane[0]->age << endl;
		for(const auto& email : usersNamedJane[0]->emails)
			cout << "email:" << email << endl;
		cout << "============" <<endl;

		const auto usersByEmail = session.query<user>()
										 ->where_in("emails",
											 vector<string>
											 {
                                                 "john.doe@example.com", 
                                                 "jane.doe@example.com"
											 })
		                                 ->to_list();		

		cout << "============" <<endl;
		cout << "query #2 results" <<endl;
		for(const auto& user : usersByEmail)
			cout << "name:" << user->name << endl;

		const auto users_complex = session.query<user>()
			->where_greater_than("age", 20)
			  ->and_also()
		    ->where_ends_with("name", "Doe")
			->to_list();

		cout << "============" <<endl;
		cout << "query #3 results" <<endl;
		for(const auto& user : users_complex)
			cout << "name:" << user->name << endl;
		const auto users_uber_complex = session.query<user>()
		    ->where_in("emails",
			  vector<string>
			  {
                "john.doe@example.com", 
                "jane.doe@example.com"
              })
		    ->or_else() // OR relationship to other clauses
			->open_subclause() //open parenthesis
			  ->where_greater_than("age", 25)
  			    ->and_also() // AND relationship between clauses
		      ->where_ends_with("name", "Doe")
			->close_subclause() //close parenthesis
			->to_list();

		cout << "============" <<endl;
		cout << "query #4 results" <<endl;

		//note: Jane Doe age = 24, but because of the OR clause
		//both John Doe and Jane Doe user records will be fetched
		for(const auto& user : users_uber_complex)
			cout << "name:" << user->name << endl;
		cout << "============" <<endl;
	}

 	return 0;
}

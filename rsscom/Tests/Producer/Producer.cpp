#include <stdexcept>
#include <iostream>
#include "Producer.h"
#include "Configuration.h"

using std::string;
using std::exception;
using std::getline;
using std::cin;
using std::cout;
using std::endl;

using RSSCom::Producer;
using RSSCom::Configuration;
using RSSCom::Topic;
using RSSCom::MessageBuilder;

int main(int argc, char* argv[]) {
    string brokers = "localhost:9092";
    string topic_name = "MyTopic";
    int partition_value = -1;

    // Create a message builder for this topic
    MessageBuilder builder(topic_name);

    // Get the partition we want to write to. If no partition is provided, this will be
    // an unassigned one
    if (partition_value != -1) {
        builder.partition(partition_value);
    }

    // Construct the configuration
    Configuration config = {
        { "metadata.broker.list", brokers }
    };

    // Create the producer
    Producer producer(config);

    cout << "Producing messages into topic " << topic_name << endl;

    // Now read lines and write them into kafka
    string line;
    while (getline(cin, line)) {
        // Set the payload on this builder
        builder.payload(line);

        // Actually produce the message we've built
        producer.produce(builder);
    }
    
    // Flush all produced messages
    producer.flush();
}

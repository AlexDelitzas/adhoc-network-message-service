########################################################################
# Description: Calculates the message statistics of the network devices
########################################################################

import json
import csv

id = 8448 # the ID of the device that its message statistics will be calculated
other_ids = [8449, 8450] # the IDs of the other devices in the network

session_data = []
sessions_with_specific_device = [0] * len(other_ids)
total_sessions = 0
messages_received = 0
messages_sent = 0
messages_in_queue = 0
messages_generated = 0
message_already_in_queue = 0
was_message_destination = 0

def analyze_queue_messages():
    global messages_in_queue
    filename = '../logs/queue_' + str(id) + '.txt'

    with open(filename) as csvfile:
        csvreader = csv.reader(csvfile, delimiter = ',')
        for row in csvreader:
            messages_in_queue += 1


def analyze_generated_messages():
    global messages_generated
    filename = '../logs/msg_creation_timestamps_' + str(id) + '.txt'

    with open(filename) as csvfile:
        csvreader = csv.reader(csvfile, delimiter = ',')
        for row in csvreader:
            messages_generated += 1


def analyze_session_messages():
    global total_sessions, messages_received, messages_sent, \
        messages_in_queue, messages_generated, \
        message_already_in_queue, was_message_destination

    filename = '../logs/sessions_' + str(id) + '.json'

    with open(filename) as json_file:
        session_data = json.load(json_file)['sessions']

    for session in session_data:
        if (len(session['messages_received']) == 0 and len(session['messages_sent']) == 0):
            continue
        total_sessions += 1

        paired_device = int(session['paired_device'])
        for i in range(0, len(other_ids)):
            if paired_device == other_ids[i]:
                sessions_with_specific_device[i] += 1
                break

        for msg in session['messages_received']:
            messages_received += 1
            if msg['already_exists'] == 1:
                message_already_in_queue += 1

            destination = int(msg['message'][5:9])

            if id == destination:
                was_message_destination += 1

        for msg in session['messages_sent']:
            messages_sent += 1



if __name__== "__main__":
    print('********** DEVICE %d **********' % id)

    analyze_queue_messages()
    analyze_generated_messages()
    analyze_session_messages()

    print('Total sessions: %d' % total_sessions)
    for i in range(0, len(other_ids)):
        print('Sessions with device %d: %d' % (other_ids[i], sessions_with_specific_device[i]))

    print('Messages received: %d' % messages_received)
    print('Messages sent: %d' % messages_sent)
    print('Messages in queue: %d' % messages_in_queue)
    print('Messages generated: %d' % messages_generated)
    print('Messages received and were already in queue: %d' % message_already_in_queue)
    print('Messages received and reached final destination: %d' % was_message_destination)

    print('Mean number of received messages per session: %.2f' % (messages_received / total_sessions))
    print('Mean number of messages sent per session: %.2f' % (messages_sent / total_sessions))

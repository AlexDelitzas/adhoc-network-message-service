########################################################################
# Description: This script checks the correctness of the implementation
#
# Tests:
# 1) Check if every generated message reached its final destination
# 2) Check if there are any duplicate messages in the message queue of
# each device
# 3) Check the session history among the devices. For every session
# between 2 devices, the transmitted and received messages must agree 
########################################################################

import csv
import json

ids = [8449, 8448, 8450] # list of device IDs in the network

created_messages = []
messages = []

def json_pretty_print(json_object):
    print(json.dumps(json_object, indent=2))

def read_message_queue():
    for i in range(0, len(ids)):
        id = ids[i]
        filename = '../logs/queue_' + str(id) + '.txt'

        messages.append([])
        with open(filename) as csvfile:
            csvreader = csv.reader(csvfile, delimiter = ',')
            for row in csvreader:
                messages[i].append(row[0])

def check_if_reached_destination():
    for i in range(0, len(ids)):
        id = ids[i]
        filename = '../logs/msg_creation_timestamps_' + str(id) + '.txt'

        created_messages.append([])
        with open(filename) as csvfile:
            csvreader = csv.reader(csvfile, delimiter = ',')
            for row in csvreader:
                created_messages[i].append(row[0])

    is_ok = True
    for i in range(0, len(ids)):
        id = ids[i]
        for msg in created_messages[i]:
            receiver = int(msg[5:9])

            if receiver in ids:
                id_pos = ids.index(receiver)
                if not msg in messages[id_pos]:
                    print('Message did not reach destination: ' + msg)
                    is_ok = False

    return is_ok

def check_for_duplicates():
    is_ok = True
    for i in range(0, len(ids)):
        if len(set(messages[i])) != len(messages[i]):
            print('Multiple occurences of the same message for device ' + str(ids[i]))
            is_ok = False

    return is_ok

def check_sessions_messages():
    session_data = []
    for i in range(0, len(ids)):
        filename = '../logs/sessions_' + str(ids[i]) + '.json'
        session_data.append([])
        with open(filename) as json_file:
            session_data[i] = json.load(json_file)['sessions']

    is_ok = True
    for i in range(0, len(ids)-1):
        for j in range(i+1, len(ids)):
            print('Check device %d with %d' % (ids[i], ids[j]))
            if i == j:
                continue

            sessions_with_paired_device1 = []
            for session in session_data[i]:
                if session['paired_device'] == ids[j]:
                    sessions_with_paired_device1.append(session)

            sessions_with_paired_device1 = sorted(sessions_with_paired_device1, key=lambda k: k["started_at"], reverse=False)

            sessions_with_paired_device2 = []
            for session in session_data[j]:
                if session['paired_device'] == ids[i]:
                    sessions_with_paired_device2.append(session)

            sessions_with_paired_device2 = sorted(sessions_with_paired_device2, key=lambda k: k["started_at"], reverse=False)

            if len(sessions_with_paired_device1) != len(sessions_with_paired_device2):
                print('Sessions number does not match')
                is_ok = False
            for session_id in range(0, len(sessions_with_paired_device1)):
                messages_sent1 = sessions_with_paired_device1[session_id]['messages_sent']
                messages_received1 = sessions_with_paired_device1[session_id]['messages_received']
                messages_sent2 = sessions_with_paired_device2[session_id]['messages_sent']
                messages_received2 = sessions_with_paired_device2[session_id]['messages_received']

                if (len(messages_sent1) != len(messages_received2)) or (len(messages_sent2) != len(messages_received1)):
                    print('Session messages are different in length')
                    is_ok = False

                for msg_id in range(0, len(messages_sent1)):
                    if messages_sent1[msg_id]["message"] != messages_received2[msg_id]["message"]:
                        print('Session messages are different')
                        is_ok = False
                for msg_id in range(0, len(messages_sent2)):
                    if messages_sent2[msg_id]["message"] != messages_received1[msg_id]["message"]:
                        print('Session messages are different')
                        is_ok = False
    return is_ok


if __name__== "__main__":
    read_message_queue()

    print('Tests:\n')

    is_ok = check_if_reached_destination()
    if is_ok == True:
        print('[*] Messages reached their destination - PASSED')
    else:
        print('[*] Messages reached their destination - FAILED')
    print('\n')

    is_ok = check_for_duplicates()
    if is_ok == True:
        print('[*] No duplicates in message queue - PASSED')
    else:
        print('[*] No duplicates in message queue - FAILED')
    print('\n')

    is_ok = check_sessions_messages()
    if is_ok == True:
        print('[*] Message exchanges during sessions were successful - PASSED')
    else:
        print('[*] Message exchanges during sessions were successful - FAILED')

#####################################################################
# Description: Calculates the time statistics of the network devices
#####################################################################

import json
import csv
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

id = 8448 # the ID of the device that its time statistics will be calculated
other_ids = [8449, 8450] # the IDs of the other devices in the network

message_generation_interval = []
time_sessions_started = []
session_duration = []

def get_message_generation_interval():

    filename = '../logs/msg_creation_timestamps_' + str(id) + '.txt'

    with open(filename) as csvfile:
        csvreader = csv.reader(csvfile, delimiter = ',')
        for row in csvreader:
            message_generation_interval.append(float(row[1]))

def get_session_times():
    filename = '../logs/sessions_' + str(id) + '.json'

    for i in range(0, len(other_ids)):
        time_sessions_started.append([])

    with open(filename) as json_file:
        session_data = json.load(json_file)['sessions']

    for session in session_data:
        if (len(session['messages_received']) == 0 and len(session['messages_sent']) == 0):
            continue

        session_duration.append(session['duration_msec'])

        paired_device_index = other_ids.index(int(session['paired_device']))
        time_sessions_started[paired_device_index].append(session['started_at'])


if __name__== "__main__":
    print('********** DEVICE %d **********' % id)
    get_message_generation_interval()
    get_session_times()

    print('Messages generated: %d' % len(message_generation_interval))
    message_generation_interval = np.array(message_generation_interval)


    interval_mean = np.mean(message_generation_interval)
    interval_std = np.std(message_generation_interval)
    interval_min = np.amin(message_generation_interval)
    interval_max = np.amax(message_generation_interval)

    print('----- Message generation interval -----')
    print('mean= %.2f sec' % interval_mean)
    print('std= %.2f sec' % interval_std)
    print('min= %.2f sec' % interval_min)
    print('max= %.2f sec' % interval_max)
    print('---------------------------------------\n\n')

    session_duration = np.array(session_duration)
    duration_mean = np.mean(session_duration)
    duration_std = np.std(session_duration)
    duration_min = np.amin(session_duration)
    duration_max = np.amax(session_duration)

    print('----- Session duration -----')
    print('mean= %.2f msec' % duration_mean)
    print('std= %.2f msec' % duration_std)
    print('min= %.2f msec' % duration_min)
    print('max= %.2f msec' % duration_max)
    print('----------------------------\n\n')

    fig = plt.figure()
    sns.set(color_codes=True)
    plt.hist(x=message_generation_interval, rwidth=0.9, color='#607c8e', bins=12)

    plt.xlabel('Message generation interval (sec)')
    plt.ylabel('Frequency')
    plt.title('Device %d' % id)
    fig.savefig('message_gen_' + str(id) + '.eps', bbox_inches='tight', dpi=1000)
    plt.show()

    fig = plt.figure()
    sns.set(color_codes=True)
    plt.hist(x=session_duration, rwidth=0.9, color='#607c8e', bins=15)

    plt.xlabel('Session duration (msec)')
    plt.ylabel('Frequency')
    plt.title('Device %d' % id)
    fig.savefig('session_duration_' + str(id) + '.eps', bbox_inches='tight', dpi=1000)
    plt.show()


    fig = plt.figure()
    sns.set(color_codes=True)

    for i in range(0, len(other_ids)):
        x = np.array(time_sessions_started[i])
        y = np.array([i/2+1] * len(time_sessions_started[i]))
        plt.scatter(x=x, y=y, label=other_ids[i], edgecolors='k')

    plt.xlabel('Time (sec)')

    plt.yticks([])
    plt.title('Device %d' % id)
    plt.legend(title='Paired device')
    fig.savefig('session_times_' + str(id) + '.eps', bbox_inches='tight', dpi=1000)
    plt.show()

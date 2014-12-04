#! /bin/bash


#=====================================================
#   Handy but kind of odd little utility functions.
#=====================================================

current_message=" "

function message
{
  echo "nightly: $1 ${current_message}"
}


function start_msg
{
  current_message=$1
  message "start: "
}


function done_msg
{
  message "done: "
}





#=====================================================
#  Handy variables (actually constants).
#=====================================================

NIGHTLY_ROOT=/home/mick/git/proton_c_clients/nightly

PSEND=${NIGHTLY_ROOT}/../psend/psend
PRECV=${NIGHTLY_ROOT}/../precv/precv
STATS=${NIGHTLY_ROOT}/utils/stats/stats
SVG=${NIGHTLY_ROOT}/utils/svg_graph/svg_graph

N_TESTS=10
DATE=`date +%Y_%m_%d`
HOST=0.0.0.0
PORT=5801
N_MSGS=5000000
MSG_LEN=100
N_LINKS=5
FREQ=1000000
INIT_FLOW=400
FLOW_INC=200
RESULTS=./results
PRECV_OUT=./precv.out
PSEND_OUT=./psend.out





#=====================================================
#  Let's Get Started !
#=====================================================



cd ${NIGHTLY_ROOT}

rm -f ${RESULTS}

echo -n "grand start: " ; date

count=0
while [ $count -lt ${N_TESTS} ]
do
  pkill precv
  pkill psend
  rm -f ${PRECV_OUT} ${PSEND_OUT}
  count=$(( $count + 1 ))

  echo "==========================================="
  echo "TEST $count"
  echo "==========================================="

  ${PRECV}                             \
    --host                ${HOST}      \
    --port                ${PORT}      \
    --messages            ${N_MSGS}    \
    --report_frequency    ${FREQ}      \
    --initial_flow        ${INIT_FLOW} \
    --flow_increment      ${FLOW_INC}  \
    --output              ${PRECV_OUT}    &

  PRECV_PID=$!

  sleep 2

  ${PSEND}                        \
    --host            ${HOST}     \
    --port            ${PORT}     \
    --messages        ${N_MSGS}   \
    --message_length  ${MSG_LEN}  \
    --n_links         ${N_LINKS}  \
    --output          ${PSEND_OUT} 2> /dev/null   &

  PSEND_PID=$!

  # Wait for both of those to finish.
  wait ${PRECV_PID}  ${PSEND_PID}

  STOP_TIME=`cat ${PRECV_OUT}  | grep precv_stop  | awk '{print $2}'`
  START_TIME=`cat ${PSEND_OUT} | grep psend_start | awk '{print $2}'`
  TOTAL_TIME=`echo ${STOP_TIME} - ${START_TIME} | bc`

  echo "total time: ${TOTAL_TIME}"
  echo ${TOTAL_TIME} >> ${RESULTS}
done
echo -n "grand stop: " ; date

done_msg "$msg"


STATS_FILE="statistics_${DATE}"
start_msg "calculating statistics"
${STATS} < ${RESULTS} > ${STATS_FILE}
done_msg

MEAN=`cat ${STATS_FILE} | awk '{print $6}'`
SIGMA=`cat ${STATS_FILE} | awk '{print $8}'`

echo ${MEAN}      > ./test_results/${DATE}
echo "  ${SIGMA}" >> ./test_results/${DATE}

start_msg "creating svg"
${SVG} ./test_results
done_msg


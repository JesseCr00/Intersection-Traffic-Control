#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <chrono>
#include <unistd.h>
#include <iomanip>
#include <ctime>
#include <thread>
#include <vector>
#include <algorithm>

using namespace std;

#define DISP 1

int main(int argc, char *argv[])
{
    int timeInterval = 10; // Cycle IRL this would really be 1 second, so that 2 intervalsis timeout time
    int iterationCount = 1;
    int max_its = 8;
    int size, ncars, my_rank;
    int conflicts[8][4] = {{2, 5, 6, 7}, {2, 3, 4, 7}, {5, 7, 0, 1}, {4, 5, 6, 1}, {6, 1, 2, 3}, {6, 7, 0, 3}, {0, 3, 4, 5}, {0, 1, 2, 5}};
    // int lsl[8][3] = {{6, 7, 0}, {1, 2, 3}, {0, 1, 2}, {3, 4, 5}, {2, 3, 4}, {5, 6, 7}, {4, 5, 6}, {7, 0 ,1}};
    int strongConc[8][2] = {{4, 1}, {0, 5}, {6, 3}, {2, 7}, {0, 5}, {1, 4}, {2, 7}, {3, 6}};
    /* start up initial MPI environment */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int recv_data[3] = {-1, -1, -1};
    int recv_data2[3] = {-1, -1, -1};
    /* process command line arguments*/
    if (argc == 2)
    {
        ncars = atoi(argv[1]);
        // dims[0] = nrows; /* number of rows */
        if (ncars != size)
        {
            if (my_rank == 0)
                cout << "ERROR: ncars = " << ncars << " != " << size << endl;
            MPI_Finalize();
            return 0;
        }
    }
    else
    {
        ncars = (int)sqrt(size - 1);
    }

    // MPI_Request send_request[ncars];
    // MPI_Request receive_request[ncars];
    MPI_Request receive_req[ncars];
    MPI_Request receive_req2[ncars];
    // MPI_Request send_request[ncars];
    // MPI_Request send_request2[ncars];

    // Initialise cars at intersection
    // Have some waiting and some idle 0 = idle, 1=waiting, 2= passing
    int carStatus = 0;
    int carLane;
    vector<int> HL;
    int HLLen = 0;
    int LLLen = 0;
    vector<int> LL;
    vector<int> HLLane; // int HLLane[ncars];
    vector<int> HLPreempt;
    vector<int> LLLane; // int LLLane[ncars];
    int CntPmp = 0;
    int TH = 3;
    int NF = 3;
    int follower = 0;
    int followLast = 0;
    int timeout = 2;
    int timeCounter = 0;
    int initBcastDone = 0;
    int rejected = 0;
    int justBcasted = 0;
    int rejectListens;

    // MPI_Comm waiting_comm;
    // int subGFlag = (carStatus != 0);
    // MPI_Comm_split(MPI_COMM_WORLD, subGFlag?0:MPI_UNDEFINED, my_rank, &waiting_comm);

    /*************************************************************/
    /* MAIN PROGRAM LOOP */
    /*************************************************************/
    // //MPI_Barrier(MPI_COMM_WORLD);
    while (iterationCount <= max_its)
    {
        // MPI_Barrier(MPI_COMM_WORLD);
        time_t time_now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct tm *ptm = localtime(&time_now);
        // cout << put_time(ptm, "%X") << ", " << iterationCount << " Rank: " << my_rank << endl;
        //  wait until next 10sec
        if (my_rank == 0)
        {
            cout << "----------Time now: " << put_time(ptm, "%X") << endl;
        }
        ptm->tm_sec += timeInterval;
        if (my_rank == 0)
        {
            cout << "----------Waiting until: " << put_time(ptm, "%X") << endl;
        }
        this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(ptm)));
        // //MPI_Barrier(MPI_COMM_WORLD);
        // Root generates unique lanes and broadcasts
        int laneThisRound;
        int lanes[ncars];
        if (my_rank == 0)
        {
            for (int i = 0; i < ncars; i++)
            {
                srand(int(time(NULL)) ^ i);
                int tempLane = rand() % 8;
                if (i == 1)
                {
                    lanes[i] = tempLane;
                }
                else
                {
                    int verifiedUnique = 0;
                    int dupCount = 0;
                    while (!verifiedUnique)
                    {
                        for (int j = 0; j < i; j++)
                        {
                            if (tempLane == lanes[j])
                            {
                                dupCount++;
                            }
                        }
                        if (dupCount == 0)
                        {
                            lanes[i] = tempLane;
                            verifiedUnique = 1;
                        }
                        else
                        {
                            srand(int(time(NULL)) ^ i);
                            tempLane = rand() % 8;
                            dupCount = 0;
                        }
                    }
                }
            }
            // cout << lanes[0] << " " << lanes[1] << " " << lanes[2] << " " << lanes[3] << endl;
        }
        // Broadcast unique lanes for new joiners
        MPI_Scatter(lanes, 1, MPI_INT, &laneThisRound, 1, MPI_INT, 0, MPI_COMM_WORLD);
        //  If idle, then run random to see if it wil approach intersection
        if (carStatus == 0)
        {
            srand(int(time(NULL)) ^ my_rank);
            carStatus = rand() % 2;
            carLane = laneThisRound;

            // TODO unique numbers
            if (carStatus == 0)
            {
                // cout << "R: " << my_rank << " still idle" << endl;
            }
            else
            {
                // cout << "DUP? IT: " << iterationCount << " R: " << my_rank << " L: " << carLane << endl;
                initBcastDone = 0;
                timeCounter = 0;
                rejected = 0;
                CntPmp = 0;
                followLast = 0;
                HLLen = 0;
                LLLen = 0;
                HL.clear();
                LL.clear();
                HLLane.clear();
                HLPreempt.clear();
                LLLane.clear();
                // int subGFlag = (carStatus != 0);
                // MPI_Comm_split(MPI_COMM_WORLD, subGFlag?0:MPI_UNDEFINED, my_rank, &waiting_comm);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 1)
        {
            int hl0 = (HLLen > 0) ? HL[0] : -1;
            int hl1 = (HLLen > 1) ? HL[1] : -1;
            int hl2 = (HLLen > 2) ? HL[2] : -1;
            int ll0 = (LLLen > 0) ? LL[0] : -1;
            int ll1 = (LLLen > 1) ? LL[1] : -1;
            int ll2 = (LLLen > 2) ? LL[2] : -1;
            cout << "IT: " << iterationCount << " | R: " << my_rank << " w L: " << carLane << " | HL: [" << hl0 << hl1 << hl2 << "] LL: " << LLLen << "[" << ll0 << ll1 << ll2 << "] R?: " << rejected << " TC: " << timeCounter << endl;
            // Broadcast Request
            if (initBcastDone == 0)
            {
                int carInfo[2] = {my_rank, carLane}; // Vehicle No, Lane No
                for (int i = 0; i < ncars; i++)
                {
                    if (i != my_rank)
                    {
                        MPI_Send(&carInfo, 2, MPI_INT, i, iterationCount, MPI_COMM_WORLD); //, &send_request[i]);
                        // MPI_Wait(&send_request[i], MPI_STATUS_IGNORE);
                        // cout << "OO" << endl;
                    }
                }
                justBcasted = 1;
                initBcastDone = 1;
            }
            else
            {
                justBcasted = 0;
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 1)
        {
            //  Check for Permit/Timeout
            if (!justBcasted) // rejected == 1 || rejected == 0
            {
                for (int i = 0; i < ncars; i++)
                {
                    int permFlag;
                    MPI_Iprobe(i, iterationCount + 20, MPI_COMM_WORLD, &permFlag, MPI_STATUS_IGNORE);
                    if (permFlag)
                    {
                        MPI_Irecv(&recv_data, 1, MPI_INT, i, iterationCount + 20, MPI_COMM_WORLD, &receive_req[i]);
                        MPI_Wait(&receive_req[i], MPI_STATUS_IGNORE);
                        //cout << "R: " << my_rank << " got permit " << recv_data[0] << endl;
                        // cout << "HL Size: " << HLLen << endl;
                        //  Delete j from HLi
                        int deleted = 0;
                        if (HLLen > 0)
                        {

                            int index = 0;
                            while (!deleted && index < HLLen)
                            {
                                if (HL[index] == recv_data[0])
                                {
                                    // Erase bc we got a permit
                                    HL.erase(HL.begin() + index);
                                    HLLane.erase(HLLane.begin() + index);
                                    HLPreempt.erase(HLPreempt.begin() + index);
                                    deleted = 1;
                                    HLLen--;
                                    cout << "R: " << my_rank << " j: " << recv_data[0] << " removed from HL on permit" << endl;
                                }
                                index++;
                            }

                            // cout << "Delete success after permit" << endl;
                        }
                        else
                        {
                            // permits broadcasted so mightnot be in there
                        }

                        if (HLLen == 0 && deleted)
                        {
                            carStatus = 2;
                            cout << "IT: " << iterationCount << " R: " << my_rank << " PASSING L: " << carLane << " Permit + Empty HL" << endl;
                            // construct flt
                            int flt[NF] = {-1, -1, -1};
                            // assuming that vehicles added to LL in reverse order of arrival
                            int counter = 0;
                            for (int k = 0; k < LLLen; k++)
                            {
                                if (carLane == LLLane[k] && counter < NF)
                                {
                                    flt[counter] = LL[k];
                                    counter++;
                                }
                            }
                            // BROADCAST FOLLOW i, flt
                            int followInfo[2] = {my_rank, carLane};
                            for (int j = 0; j < ncars; j++)
                            {
                                if (j != my_rank)
                                {
                                    MPI_Send(&followInfo, 2, MPI_INT, j, iterationCount + 30, MPI_COMM_WORLD); //, &send_request[j]);
                                    MPI_Send(&flt, NF, MPI_INT, j, iterationCount + 31, MPI_COMM_WORLD);       //, &send_request2[j]);
                                }
                            }
                        }
                    }
                } // PASS CORE AREA TODO?
            }
        }
        // CHECK FOR REQUEST
        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 1)
        {
            for (int i = 0; i < ncars; i++)
            {
                int reqFlag;
                MPI_Iprobe(i, iterationCount, MPI_COMM_WORLD, &reqFlag, MPI_STATUS_IGNORE);
                if (reqFlag)
                {
                    // Receives j, j lane
                    MPI_Irecv(&recv_data, 2, MPI_INT, i, iterationCount, MPI_COMM_WORLD, &receive_req[i]);
                    MPI_Wait(&receive_req[i], MPI_STATUS_IGNORE);
                    // cout << "R: " << my_rank << " GOT REQ FROM R: " << recv_data[0] << endl;
                    // cout << "XX" << endl;
                    int conf = 0;
                    for (int j = 0; j < 4; j++)
                    {
                        if (conflicts[carLane][j] == recv_data[1])
                        {
                            conf = 1;
                        }
                    }
                    if ((carStatus == 1 || carStatus == 2) && ((carLane == recv_data[1]) || (conf)))
                    {
                        if (conf && justBcasted && my_rank < recv_data[0])
                        {
                            // Dont receive this req, 2 conflict at same exact time, one technically will not receive req
                            // cout << "Success time sim. R: " << my_rank << " gave way to R: " << recv_data[0] << endl;
                        }
                        else
                        {
                            // Is j strong concurrent with k
                            int HLStrongConc = 0;
                            for (int j = 0; j < HLLen; j++)
                            {
                                for (int k = 0; k < 2; k++)
                                {
                                    if (HLLane[j] == strongConc[recv_data[1]][k]) // in HL are any strong conc with requester
                                    {
                                        HLStrongConc = 1;
                                        // cout << "R: " << my_rank << " (req) strong conc " << HL[j] << endl;
                                    }
                                }
                            }
                            if ((HLStrongConc) && (CntPmp < TH))
                            {
                                int HLdone = 0;
                                for (int m = 0; m < HLLen; m++)
                                {
                                    if (HL[m] == recv_data[0])
                                    {
                                        HLdone = 1;
                                    }
                                }

                                if (!HLdone)
                                {
                                    HL.insert(HL.begin(), recv_data[0]);
                                    cout << "R: " << my_rank << " (pre emp) ADD TO HL R: " << recv_data[0] << endl;
                                    HLLane.insert(HLLane.begin(), recv_data[1]);
                                    HLPreempt.insert(HLPreempt.begin(), 1);
                                    HLLen++;
                                    CntPmp++;
                                    for (int m = 0; m < LLLen; m++)
                                    {
                                        if (LL[m] == recv_data[0])
                                        {
                                            // Erase only bc we put it in HL
                                            LL.erase(LL.begin() + m);
                                            LLLane.erase(LLLane.begin() + m);
                                            LLLen--;
                                            // cout << "R: " << my_rank << " j: " << recv_data[0] << " removed from LL on add to HL" << endl;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                int LLdone = 0;
                                for (int m = 0; m < LLLen; m++)
                                {
                                    if (LL[m] == recv_data[0])
                                    {
                                        LLdone = 1;
                                    }
                                }

                                if (!LLdone)
                                {
                                    LL.insert(LL.begin(), recv_data[0]);
                                    // cout << "BROD REJ: " << my_rank << " FOR" << recv_data[0] << endl;
                                    LLLane.insert(LLLane.begin(), recv_data[1]);
                                    LLLen++;
                                    // TODO REMOVE FROM HI???
                                    for (int m = 0; m < HLLen; m++)
                                    {
                                        if (HL[m] == recv_data[0])
                                        {
                                            // Erase only bc we put it in low
                                            HL.erase(HL.begin() + m);
                                            HLLane.erase(HLLane.begin() + m);
                                            HLPreempt.erase(HLPreempt.begin() + m);
                                            HLLen--;
                                            // cout << "R: " << my_rank << " j: " << recv_data[0] << " removed from HL on add to LL" << endl;
                                        }
                                    }
                                }
                                // BROADCAST REJECT I, J, Ilane
                                int carInfo[3] = {my_rank, recv_data[0], carLane};
                                for (int j = 0; j < ncars; j++)
                                {
                                    if (j != my_rank)
                                    {
                                        MPI_Send(&carInfo, 3, MPI_INT, j, iterationCount + 10, MPI_COMM_WORLD); //, &send_request[j]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 1)
        {
            //  Check for rejection
            for (int i = 0; i < ncars; i++)
            {
                int rejFlag;
                MPI_Iprobe(i, iterationCount + 10, MPI_COMM_WORLD, &rejFlag, MPI_STATUS_IGNORE);
                if (rejFlag)
                {
                    MPI_Irecv(&recv_data, 3, MPI_INT, i, iterationCount + 10, MPI_COMM_WORLD, &receive_req[i]);
                    MPI_Wait(&receive_req[i], MPI_STATUS_IGNORE);
                    // cout << "GOT REJ: " << my_rank << " FROM " << recv_data[0] << endl;
                    if (carStatus == 1)
                    {

                        if (my_rank == recv_data[1]) // become rejected, only if you already arent
                        {
                            int HLdone = 0;
                            for (int m = 0; m < HLLen; m++)
                            {
                                if (HL[m] == recv_data[0])
                                {
                                    HLdone = 1;
                                }
                            }

                            if (!HLdone)
                            {
                                HL.insert(HL.begin(), recv_data[0]);
                                cout << "R: " << my_rank << " (rejected) ADD TO HL R: " << recv_data[0] << endl;
                                HLLane.insert(HLLane.begin(), recv_data[2]);
                                HLPreempt.insert(HLPreempt.begin(), 0);
                                HLLen++;

                                for (int m = 0; m < LLLen; m++)
                                {
                                    if (LL[m] == recv_data[0])
                                    {
                                        LL.erase(LL.begin() + m);
                                        LLLane.erase(LLLane.begin() + m);
                                        LLLen--;
                                        // cout << "R: " << my_rank << " j: " << recv_data[0] << " removed from LL on add to HL" << endl;
                                    }
                                }
                            }

                            rejected = 1;

                            // cout << "SUCC REG: " << my_rank << " BY " << recv_data[0] << endl;
                        }
                        else if (my_rank != recv_data[1])
                        {
                            // See if K is in HL
                            int KinHL = 0;
                            for (int j = 0; j < HLLen; j++)
                            {
                                if (HL[j] == recv_data[1] && HLPreempt[j] == 1)
                                {
                                    KinHL = 1;
                                }
                            }
                            // See if J conflicts with I and is not in i's HL
                            int conf = 0;
                            for (int j = 0; j < 4; j++)
                            {
                                if (conflicts[carLane][j] == recv_data[2])
                                {
                                    conf = 1;
                                }
                            }
                            int JinHL = 0;
                            for (int j = 0; j < HLLen; j++)
                            {
                                if (HL[j] == recv_data[0])
                                {
                                    JinHL = 1;
                                }
                            }
                            int jiStrongConc = 0;
                            for (int j = 0; j < HLLen; j++)
                            {
                                for (int k = 0; k < 2; k++)
                                {
                                    if (recv_data[2] == strongConc[carLane][k]) // am I strong concurrent with rejecter
                                    {
                                        jiStrongConc = 1;
                                        // cout << "R: " << my_rank << " (rej) strong conc " << HL[j] << endl;
                                    }
                                }
                            }
                            if (KinHL && ((conf && !JinHL) || jiStrongConc))
                            {
                                // cout << my_rank << " about to re-emit reject by " << recv_data[0] << endl;
                                if (HLLen > 0)
                                {
                                    int deleted = 0;
                                    int index = 0;
                                    while (!deleted && index < HLLen) // TODO?
                                    {
                                        if (HL[index] == recv_data[1] && HLLen > 0)
                                        {
                                            // erase because we prempted KL etc etc
                                            HL.erase(HL.begin() + index);
                                            HLLane.erase(HLLane.begin() + index);
                                            HLPreempt.erase(HLPreempt.begin() + index);
                                            // cout << "R: " << my_rank << " k: " << recv_data[1] << " removed from HL on reject/reemit" << endl;
                                            deleted = 1;
                                            HLLen--;
                                            if (HLLen == 0)
                                            {
                                                rejected = 0;
                                            }
                                        }
                                        index++;
                                    }
                                }
                                else
                                {
                                    cout << " THIS CANNOT HAPPEN, K should be in HL" << endl;
                                }
                                // BROADCAST REJECT I K, I lane // TODO used to be j lane
                                int carInfo[3] = {my_rank, recv_data[1], carLane}; // Vehicle No, Lane No
                                for (int j = 0; j < ncars; j++)
                                {
                                    if (j != my_rank)
                                    {
                                        MPI_Send(&carInfo, 3, MPI_INT, j, iterationCount + 10, MPI_COMM_WORLD); //, &send_request[j]);
                                    }
                                }
                                rejectListens++;
                            }
                        }
                    }
                }
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 1)
        {
            if (timeCounter >= timeout && rejected == 0)
            {
                if (HLLen == 0)
                {
                    carStatus = 2;
                    cout << "IT: " << iterationCount << " R: " << my_rank << " PASSING L: " << carLane << " T.O" << endl;
                    // construct flt
                    int flt[NF] = {-1, -1, -1};
                    // assuming that vehicles added to LL in reverse order of arrival
                    int counter = 0;
                    for (int k = 0; k < LLLen; k++)
                    {
                        if (carLane == LLLane[k] && counter < NF)
                        {
                            flt[counter] = LL[k];
                            counter++;
                        };
                    }
                    // cout << "FLT R: " << my_rank << " " << flt[0] << " " << flt[1] << " " << flt[2] << endl;
                    //  BROADCAST FOLLOW i, flt
                    int followInfo[2] = {my_rank, carLane}; // TODO
                    for (int j = 0; j < ncars; j++)
                    {
                        if (j != my_rank)
                        {
                            MPI_Send(&followInfo, 2, MPI_INT, j, iterationCount + 30, MPI_COMM_WORLD); //, &send_request[j]);
                            MPI_Send(&flt, NF, MPI_INT, j, iterationCount + 31, MPI_COMM_WORLD);       //, &send_request2[j]);
                        }
                    }
                }
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 1)
        {
            // Check for Follow
            for (int i = 0; i < ncars; i++)
            {
                int followFlag;
                int followFlag2;
                MPI_Iprobe(i, iterationCount + 30, MPI_COMM_WORLD, &followFlag, MPI_STATUS_IGNORE);
                MPI_Iprobe(i, iterationCount + 31, MPI_COMM_WORLD, &followFlag2, MPI_STATUS_IGNORE);
                if (followFlag && followFlag2)
                {
                    MPI_Irecv(&recv_data, 2, MPI_INT, i, iterationCount + 30, MPI_COMM_WORLD, &receive_req[i]);
                    MPI_Wait(&receive_req[i], MPI_STATUS_IGNORE);
                    MPI_Irecv(&recv_data2, NF, MPI_INT, i, iterationCount + 31, MPI_COMM_WORLD, &receive_req2[i]);
                    MPI_Wait(&receive_req2[i], MPI_STATUS_IGNORE);
                    // cout << "IT: " << iterationCount << " R: " << my_rank << " recv follow from " << recv_data[0] << endl;
                    for (int k = 0; k < NF; k++)
                    {
                        if (recv_data2[k] == my_rank)
                        {
                            carStatus = 2;
                            follower = 1;
                            HL.clear();
                            HLLen = 0;
                            HLLane.clear();
                            HLPreempt.clear();
                            cout << "IT: " << iterationCount << " R: " << my_rank << " followed " << recv_data[0] << endl;
                            if (k == NF - 1 || recv_data2[k + 1] == -1)
                            {
                                // cout << "Assigned followlast R: " << my_rank << endl;
                                followLast = 1;
                            }
                            // MOVE AND PASS CORE AREA;
                        }
                    }
                    int conf = 0;
                    for (int j = 0; j < 4; j++)
                    {
                        if (conflicts[carLane][j] == recv_data[1]) // do i and j conflict
                        {
                            conf = 1;
                        }
                    }
                    if (carStatus != 2 && conf)
                    {
                        // cout << "Follow else success" << endl;
                        //  Delete j from HLi
                        int deleted = 0;
                        int index = 0;
                        if (HLLen > 0)
                        {
                            while (!deleted && index < HLLen)
                            {
                                if (HL[index] == recv_data[0])
                                {
                                    // Erase because j sent follow, we know it is passing
                                    HL.erase(HL.begin() + index);
                                    HLLane.erase(HLLane.begin() + index);
                                    HLPreempt.erase(HLPreempt.begin() + index);
                                    cout << "R: " << my_rank << " j: " << recv_data[0] << " removed from HL on follow/conf index: " << index << endl;
                                    HLLen--;
                                    deleted = 1;
                                }
                                index++;
                            }
                            // int hl0 = (HLLen > 0) ? HL[0] : -1;
                            // int hl1 = (HLLen > 1) ? HL[1] : -1;
                            // int hl2 = (HLLen > 2) ? HL[2] : -1;
                            // int ll0 = (LLLen > 0) ? LL[0] : -1;
                            // int ll1 = (LLLen > 1) ? LL[1] : -1;
                            // int ll2 = (LLLen > 2) ? LL[2] : -1;
                            // cout << "IT: " << iterationCount << " R: " << my_rank << " HL: " << HLLen << "[" << hl0 << hl1 << hl2 << "] LL: " << LLLen << "[" << ll0 << ll1 << ll2 << "]" << endl;
                        }

                        else
                        {
                            // cout << "HLLen 0 R: " << my_rank << endl;
                        }
                        // Delete flt vehicles from HL and LL
                        if (recv_data2[0] != -1)
                        {
                            for (int k = 0; k < NF; k++)
                            {
                                // int index = 0;
                                for (int m = 0; m < HLLen; m++)
                                {
                                    if (HL[m] == recv_data2[k] && recv_data2[k] != -1)
                                    {
                                        // Erase bc it was in flt
                                        HL.erase(HL.begin() + m);
                                        HLLane.erase(HLLane.begin() + m);
                                        HLPreempt.erase(HLPreempt.begin() + m);
                                        HLLen--;
                                        // cout << "R: " << my_rank << " fltitem: " << recv_data2[m] << " removed from HL on follow/conf" << endl;
                                    }
                                }
                                for (int m = 0; m < LLLen; m++)
                                {
                                    if (LL[m] == recv_data2[k] && recv_data2[k] != -1)
                                    {
                                        // Erase bc it was in flt
                                        LL.erase(LL.begin() + m);
                                        LLLane.erase(LLLane.begin() + m);
                                        LLLen--;
                                        // cout << "R: " << my_rank << " fltitem: " << recv_data2[m] << " removed from LL on follow/conf" << endl;
                                    }
                                }
                            }
                        }

                        // Add last flt vehicle to HL
                        if (recv_data2[0] != -1)
                        {
                            for (int m = 0; m < NF; m++)
                            {
                                if (((m == NF - 1) && (recv_data2[m] != -1)) || (recv_data2[m] != -1 && recv_data2[m + 1] == -1))
                                {
                                    // is it already in there
                                    int HLdone = 0;
                                    for (int b = 0; b < HLLen; b++)
                                    {
                                        if (HL[b] == recv_data2[m])
                                        {
                                            HLdone = 1;
                                        }
                                    }

                                    if (!HLdone)
                                    {
                                        HL.insert(HL.begin(), recv_data2[m]);
                                        //cout << "FLT: " << recv_data2[0] << " " << recv_data2[1] << " " << recv_data2[2] << endl;
                                        //cout << "Last FLT vehicle, " << recv_data2[m] << " added to " << my_rank << " HL Sender: " << recv_data[0] << endl;
                                        HLLane.insert(HLLane.begin(), recv_data[1]); // TODO
                                        HLPreempt.insert(HLPreempt.begin(), 0);
                                        HLLen++;
                                        for (int m = 0; m < LLLen; m++)
                                        {
                                            if (LL[m] == recv_data2[m])
                                            {
                                                // Erase only bc we put it in HL
                                                LL.erase(LL.begin() + m);
                                                LLLane.erase(LLLane.begin() + m);
                                                LLLen--;
                                                // cout << "R: " << my_rank << " j: " << recv_data2[m] << " removed from LL on add to HL" << endl;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (HLLen == 0)
                        {
                            rejected = 0;
                        }
                    }
                }
            }
            timeCounter++;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if (carStatus == 2)
        {
            if (!(follower && !followLast))
            {
                // BROADCAST Permit I
                int permit = my_rank;
                for (int j = 0; j < ncars; j++)
                {
                    if (j != my_rank)
                    {
                        MPI_Send(&permit, 1, MPI_INT, j, iterationCount + 21, MPI_COMM_WORLD); //, &send_request[j]);
                    }
                }
                //cout << "IT: " << iterationCount << " R: " << my_rank << " sent permits" << endl;
                // cout << "IT: " << iterationCount << " R: " << my_rank << " sending permits" << endl;
            }
            if (carStatus == 2)
            {
                cout << "IT: " << iterationCount << " R: " << my_rank << " passed, now idle" << endl;
            }
            carStatus = 0;
        }

        iterationCount++;
        MPI_Barrier(MPI_COMM_WORLD);
    }
    cout.flush();

    MPI_Finalize();
    return 0;
}

/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */


#include "NattyDBOperator.h"
#include "NattyRBTree.h"
#include "NattyResult.h"

#include <string.h>
#include <wchar.h>

#if ENABLE_NTY_CONNECTION_POOL
static nException Exception_frame = {"NattyDBOperator"};
#endif

void* ntyConnectionPoolInitialize(ConnectionPool *pool) {
#if ENABLE_NTY_CONNECTION_POOL
	pool->nPool = malloc(sizeof(nConnPool));

	pool->nPool = ntyConnPoolInitialize(pool->nPool);
#else
	pool->url = URL_new(MYSQL_DB_CONN_STRING);
	pool->nPool = ConnectionPool_new(pool->url);

	ConnectionPool_setInitialConnections(pool->nPool, 20);
    ConnectionPool_setMaxConnections(pool->nPool, 100);
    ConnectionPool_setConnectionTimeout(pool->nPool, 2);
    ConnectionPool_setReaper(pool->nPool, 1);
	
	ConnectionPool_start(pool->nPool);
#endif
	return pool;
}

void* ntyConnectionPoolDestory(ConnectionPool *pool) {
#if ENABLE_NTY_CONNECTION_POOL

	pool->nPool = ntyConnPoolRelease(pool->nPool);

#else
	ConnectionPool_free(&pool->nPool);
	ConnectionPool_stop(pool->nPool);
	URL_free(&pool->url);

	pool->nPool = NULL;
	pool->url = NULL;
	pool = NULL;
#endif
	return pool;
}

void *ntyConnectionPoolCtor(void *self) {
	return ntyConnectionPoolInitialize(self);
}

void *ntyConnectionPoolDtor(void *self) {
	return ntyConnectionPoolDestory(self);
}


static const ConnectionPoolHandle ntyConnectionPoolHandle = {
	sizeof(ConnectionPool),
	ntyConnectionPoolCtor,
	ntyConnectionPoolDtor,
	NULL,
	NULL,
};

const void *pNtyConnectionPoolHandle = &ntyConnectionPoolHandle;
static void *pConnectionPool = NULL;

void *ntyConnectionPoolInstance(void) {
	if (pConnectionPool == NULL) {
		void *pCPool = New(pNtyConnectionPoolHandle);
		if ((unsigned long)NULL != cmpxchg((void*)(&pConnectionPool), (unsigned long)NULL, (unsigned long)pCPool, WORD_WIDTH)) {
			Delete(pCPool);
		}
	}
#if 1 //ConnectionPool is Full
	if(ntyConnectionPoolDynamicsSize(pConnectionPool)) {
		return NULL;
	}
#endif
	return pConnectionPool;
}

static void *ntyGetConnectionPoolInstance(void) {
	return pConnectionPool;
}

int ntyConnectionPoolDynamicsSize(void *self) {
	ConnectionPool *pool = self;
	int size = ConnectionPool_size(pool->nPool);
	int active = ConnectionPool_active(pool->nPool);
	int max = ConnectionPool_getMaxConnections(pool->nPool);
	
#if (ENABLE_NTY_CONNECTION_POOL == 0)
	if (size > max * CONNECTION_SIZE_THRESHOLD_RATIO) {
		int n = ConnectionPool_reapConnections(pool->nPool);
		if (n < max * CONNECTION_SIZE_REAP_RATIO) {
			ntylog(" Connection Need to Raise Connection Size\n");
		}
	}
#if 0
	if (size >= max*CONNECTION_SIZE_THRESHOLD_RATIO || active >= max*CONNECTION_SIZE_REAP_RATIO) {
		return 1;
	}
#endif	
#endif
	ntylog(" size:%d, active:%d, max:%d\n", size, active, max);
	return 0;
}

void ntyConnectionPoolRelease(void *self) {	
	Delete(self);
	pConnectionPool = NULL;
}



#if 0
int ntyConnectionPoolExecute(void *_self, U8 *sql) {
	ConnectionPool *pool = _self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
		
	TRY 
	{
		Connection_execute(con, sql);
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}


int ntyConnectionPoolQuery(void *_self, U8 *sql, void ***result, int *length) {
	ConnectionPool *pool = _self;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
		

	TRY 
	{
		ResultSet_T r = Connection_executeQuery(con, sql);
		while (ResultSet_next(r)) {
			
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		Connection_close(con);
	}
	END_TRY;

	return ret;
}
#endif


Connection_T ntyCheckConnection(void *self, Connection_T con) {
#if 0
	ConnectionPool *pool = self;
	if (pool == NULL) return con;
	if (con == NULL) {

		int n = ConnectionPool_reapConnections(pool->nPool);
		Connection_T con = ConnectionPool_getConnection(pool->nPool);
		return NULL;

	}
#endif

	return con;
}

void ntyConnectionClose(Connection_T con) {
	
	if (con != NULL) {
		Connection_close(con);
	}
}

//NTY_DB_WATCH_INSERT_FORMAT

static int ntyExecuteWatchInsert(void *self, U8 *imei) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	
	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_WATCH_INSERT_FORMAT, imei);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_APPIDLIST_SELECT_FORMAT
static int ntyQueryAppIDListSelect(void *self, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_APPIDLIST_SELECT_FORMAT, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					C_DEVID id = ResultSet_getLLong(r, 1);
					ntylog("app: %lld\n", id);
#if 1
					ntyVectorInsert(container, &id, sizeof(C_DEVID));
#else
					ntyFriendsTreeInsert(tree, id);
#endif
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_WATCHIDLIST_SELECT_FORMAT
static int ntyQueryWatchIDListSelect(void *self, C_DEVID aid, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_WATCHIDLIST_SELECT_FORMAT, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					C_DEVID id = ResultSet_getLLong(r, 1);
#if 1
					ntylog(" ntyQueryWatchIDListSelect : %lld\n", id);
					ntyVectorInsert(container, &id, sizeof(C_DEVID));
#else
					ntyFriendsTreeInsert(tree, id);
#endif
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEV_APP_INSERT_FORMAT
static int ntyQueryDevAppRelationInsert(void *self, C_DEVID aid, U8 *imei) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEV_APP_INSERT_FORMAT, aid, imei);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
				}
				ret = 0;
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEV_APP_DELETE_FORMAT
static int ntyExecuteDevAppRelationDelete(void *self, C_DEVID aid, C_DEVID did) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEV_APP_DELETE_FORMAT, aid, did);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_GROUP
static int ntyQueryDevAppGroupInsert(void *self, C_DEVID aid, C_DEVID imei, U8 *name) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_GROUP, imei, aid, name);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_CHECK_GROUP
static int ntyQueryDevAppGroupCheckSelect(void *self, C_DEVID aid, C_DEVID imei) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ntylog(" ntyQueryDevAppGroupCheckSelect ...\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_CHECK_GROUP, imei, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
					ntylog(" ntyQueryDevAppGroupCheckSelect ...\n");
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_BIND_GROUP
static int ntyExecuteDevAppGroupBindInsert(void *self, int msgId, C_DEVID *proposerId, U8 *phonenum, int *pid, char *pname, char *pimage) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
		
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_BIND_GROUP, msgId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
					
					*proposerId = ResultSet_getLLong(r, 2);
					const char *num = ResultSet_getString(r, 3);
					ntylog("ntyExecuteDevAppGroupBindInsert --> num:%s, proposerId:%lld\n", num, *proposerId);
					if (num != NULL) {
						memcpy(phonenum, num, strlen(num));
					}

					*pid = ResultSet_getInt(r, 4);

					const char *r_name = ResultSet_getString(r, 5);
					if (r_name != NULL) {
						size_t name_len = strlen(r_name);
						memcpy(pname, r_name, name_len);
						ntylog("ntyExecuteDevAppGroupBindAndAgreeInsert --> r_name:%s\n", pname);
					}

					const char *r_image = ResultSet_getString(r, 6);
					if (r_image != NULL) {
						size_t image_len = strlen(r_image);
						memcpy(pimage, r_image, image_len);
						ntylog("ntyExecuteDevAppGroupBindAndAgreeInsert --> r_image:%s\n", pimage);
					}
					
					ntylog("ntyExecuteDevAppGroupBindInsert --> ret:%d, proposerId:%lld, pid:%d\n", ret, *proposerId, *pid);
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_INSERT_BIND_AGREE
static int ntyExecuteDevAppGroupBindAndAgreeInsert(void *self, int msgId, C_DEVID *proposerId, char *phonenum, int *pid, char *pname, char *pimage) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 sql[512] = {0};
			sprintf(sql, NTY_DB_INSERT_BIND_AGREE, msgId);
			ntylog("%s\n", sql);

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_BIND_AGREE, msgId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);
					*proposerId = ResultSet_getLLong(r, 2);

					const char *r_phonenum = ResultSet_getString(r, 3);
					if (r_phonenum != NULL) {
						size_t phonenum_len = strlen(r_phonenum);
						//phonenum = malloc(phonenum_len+1);
						//if (phonenum != NULL) {
						//	memset(phonenum, 0, phonenum_len+1);
							memcpy(phonenum, r_phonenum, phonenum_len);
							ntylog("ntyExecuteDevAppGroupBindAndAgreeInsert --> r_phonenum:%s\n", phonenum);
						//}
					}
					
					*pid = ResultSet_getInt(r, 4);

					const char *r_name = ResultSet_getString(r, 5);
					if (r_name != NULL) {
						size_t name_len = strlen(r_name);
						//pname = malloc(name_len+1);
						//if (pname != NULL) {
						//	memset(pname, 0, name_len+1);
							memcpy(pname, r_name, name_len);
							ntylog("ntyExecuteDevAppGroupBindAndAgreeInsert --> r_name:%s\n", pname);
						//}
					}

					const char *r_image = ResultSet_getString(r, 6);
					if (r_image != NULL) {
						size_t image_len = strlen(r_image);
						//pimage = malloc(image_len+1);
						//if (pimage != NULL) {
						//	memset(pimage, 0, image_len+1);
							memcpy(pimage, r_image, image_len);
							ntylog("ntyExecuteDevAppGroupBindAndAgreeInsert --> r_image:%s\n", pimage);
						//}
					}
					
					ntylog("ntyExecuteDevAppGroupBindAndAgreeInsert --> ret:%d, proposerId:%lld, pid:%d\n", ret, *proposerId, *pid);
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DELETE_BIND_GROUP
static int ntyExecuteBindConfirmDelete(void *self, int msgId, char *phonenum, C_DEVID *id) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DELETE_BIND_GROUP, msgId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*id = ResultSet_getLLong(r, 1);

					const char *r_phonenum = ResultSet_getString(r, 2);
					if (r_phonenum != NULL) {
						size_t phonenum_len = strlen(r_phonenum);
						memcpy(phonenum, r_phonenum, phonenum_len);
					}

					ntylog("ntyExecuteBindConfirmDelete -->  id:%lld\n", *id);
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DELETE_COMMON_OFFLINE_MSG
static int ntyExecuteCommonOfflineMsgDelete(void *self, int msgId, C_DEVID clientId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DELETE_COMMON_OFFLINE_MSG, msgId, clientId);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SELECT_COMMON_OFFLINE_MSG
static int ntyQueryCommonOfflineMsgSelect(void *self, C_DEVID deviceId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_COMMON_OFFLINE_MSG, deviceId);

			if (r != NULL) {
				while (ResultSet_next(r)) {
					int msgId = ResultSet_getInt(r, 1);
					C_DEVID r_senderId = ResultSet_getLLong(r, 2);
					C_DEVID r_groupId = ResultSet_getLLong(r, 3);
					const char *r_details = ResultSet_getString(r, 4);
					
					
					CommonOfflineMsg *pCommonOfflineMsg = malloc(sizeof(CommonOfflineMsg));
					if (pCommonOfflineMsg == NULL) {
						ntylog(" %s --> malloc failed CommonOfflineMsg. \n", __func__);
						break;
					}
					memset(pCommonOfflineMsg, 0, sizeof(CommonOfflineMsg));
					
					pCommonOfflineMsg->msgId = msgId;
					pCommonOfflineMsg->senderId = r_senderId;
					pCommonOfflineMsg->groupId = r_groupId;

					if (r_details != NULL) {
						size_t details_len = strlen(r_details);
						ntyCopyString(&pCommonOfflineMsg->details, r_details, details_len);
					}

					/*
					pCommonOfflineMsg->details = malloc(details_len+1);
					if (pCommonOfflineMsg->details == NULL) {
						ntylog(" %s --> malloc failed CommonOfflineMsg->details\n", __func__);
						free(pCommonOfflineMsg);
						break;
					}
					memset(pCommonOfflineMsg->details, 0, details_len+1);
					memcpy(pCommonOfflineMsg->details, r_details, details_len);
					*/
					
					ntyVectorInsert(container, pCommonOfflineMsg, sizeof(CommonOfflineMsg));
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//PROC_SELECT_VOICE_OFFLINE_MSG
static int ntyQueryVoiceOfflineMsgSelect(void *self, C_DEVID fromId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ret = -1;
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_VOICE_OFFLINE_MSG, fromId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int msgId = ResultSet_getInt(r, 1);
					C_DEVID r_senderId = ResultSet_getLLong(r, 2);
					C_DEVID r_groupId = ResultSet_getLLong(r, 3);
					const char *r_details = ResultSet_getString(r, 4);
					long timeStamp = ResultSet_getTimestamp(r, 5);

					nOfflineMsg *pOfflineMsg = malloc(sizeof(nOfflineMsg));
					if (pOfflineMsg == NULL) {
						ntylog(" %s --> malloc nOfflineMsg.details error. \n", __func__);
						break;
					}
					memset(pOfflineMsg, 0, sizeof(nOfflineMsg));
					
					pOfflineMsg->msgId = msgId;
					pOfflineMsg->senderId = r_senderId;
					pOfflineMsg->groupId = r_groupId;
					pOfflineMsg->timeStamp = timeStamp;

					if (r_details != NULL) {
						size_t details_len = strlen(r_details);
						ntyCopyString(&pOfflineMsg->details, r_details, details_len);
					}

					/*
					pOfflineMsg->details = malloc(details_len+1);
					if (pOfflineMsg->details != NULL) {
						memset(pOfflineMsg->details, 0, details_len+1);
						memcpy(pOfflineMsg->details, r_details, details_len);
					}
					*/
					
					ntyVectorInsert(container, pOfflineMsg, sizeof(CommonOfflineMsg));
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SELECT_PHONEBOOK_BINDAGREE

static int ntyQueryPhonebookBindAgreeSelect(void *self, C_DEVID did, C_DEVID proposerId, char *phonenum, int *pid, char *pname, char *pimage) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ret = -1;
			
			U8 sql[512] = {0};
			sprintf(sql, NTY_DB_SELECT_PHONEBOOK_BINDAGREE, did, phonenum, proposerId);	
			ntylog("%s\n", sql);
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_PHONEBOOK_BINDAGREE, did, phonenum, proposerId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					
					int r_id = ResultSet_getInt(r, 1);
					const char *r_name = ResultSet_getString(r, 2);
					const char *r_image = ResultSet_getString(r, 3);
					
					*pid = r_id;

					if (r_name != NULL) {
						size_t name_len = strlen(r_name);
						ntyCopyString(&pname, r_name, name_len);
						
						/*
						size_t name_len = strlen(r_name);
						pname = malloc(name_len+1);
						if (pname != NULL) {
							memset(pname, 0, name_len+1);
							memcpy(pname, r_name, name_len);
						}
						*/
					}

					if (r_image != NULL) {
						size_t image_len = strlen(r_image);
						ntyCopyString(&pimage, r_image, image_len);

						/*
						size_t image_len = strlen(r_image);
						pimage = malloc(image_len+1);
						if (pimage != NULL) {
							memset(pimage, 0, image_len+1);
							memcpy(pimage, r_image, image_len);
						}
						*/
					}
					
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_SELECT_COMMON_MSG
static int ntyQueryCommonMsgSelect(void *self, C_DEVID msgId, C_DEVID *senderId, C_DEVID *groupId, char *json) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
		/*
			U8 buffer[512];
			sprintf(buffer, NTY_DB_SELECT_COMMON_MSG, msgId);
			//ntylog(buffer);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_COMMON_MSG, msgId);
			while (ResultSet_next(r)) {
				C_DEVID r_senderId = ResultSet_getLLong(r, 1);
				C_DEVID r_groupId = ResultSet_getLLong(r, 2);
				const char *r_details = ResultSet_getString(r, 3);
				memcpy(json, r_details, strlen(r_details));
				*senderId = r_senderId;
				*groupId = r_groupId;
			}
			*/
		}
	} 
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;	

	return ret;
}


//NTY_DB_INSERT_BIND_CONFIRM
static int ntyQueryBindConfirmInsert(void *self, C_DEVID admin, C_DEVID imei, U8 *name, U8 *wimage, C_DEVID proposer, U8 *call, U8 *uimage, int *msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 sql[512] = {0};
			sprintf(sql, NTY_DB_INSERT_BIND_CONFIRM, admin, imei, name, wimage, proposer, call, uimage);			
			ntylog("%s\n", sql);	
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_BIND_CONFIRM, admin, imei, name, wimage, proposer, call, uimage);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msgId = ResultSet_getInt(r, 1);
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_SELECT_PHONE_NUMBER
static int ntyQueryPhoneBookSelect(void *self, C_DEVID imei, C_DEVID userId, char *phonenum) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_PHONE_NUMBER, imei, userId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					const char *pnum = ResultSet_getString(r, 1);
					ntylog(" ntyQueryPhoneBookSelect --> PhoneNum:%s\n", pnum);
					memcpy(phonenum, pnum, strlen(pnum));

					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_SELECT_ADMIN
static int ntyQueryAdminSelect(void *self, C_DEVID did, C_DEVID *appid) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_ADMIN, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
#if 0
					ret = ResultSet_getInt(r, 1);
#else
					*appid = ResultSet_getLLong(r, 1);
#endif
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN
static int ntyQueryBindOfflineMsgToAdminSelect(void *self, C_DEVID fromId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ret = -1;

			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN, fromId);			
			ntylog("%s\n", sql);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_ADMIN, fromId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int bId = ResultSet_getInt(r, 1);
					
					C_DEVID r_admin = ResultSet_getLLong(r, 2);
					C_DEVID r_imei = ResultSet_getLLong(r, 3);
					const char *r_watchname = ResultSet_getString(r, 4);
					const char *r_watchimage = ResultSet_getString(r, 5);
					C_DEVID r_proposer = ResultSet_getLLong(r, 6);
					const char *r_usercall = ResultSet_getString(r, 7);
					const char *r_userimage = ResultSet_getString(r, 8);
					
					BindOfflineMsgToAdmin *pMsgToAdmin = malloc(sizeof(BindOfflineMsgToAdmin));
					if (pMsgToAdmin == NULL) {
						ntylog(" %s --> malloc BindOfflineMsgToAdmin error. \n", __func__);
						break;
					}
					memset(pMsgToAdmin, 0, sizeof(BindOfflineMsgToAdmin));
					
					pMsgToAdmin->msgId = bId;
					pMsgToAdmin->IMEI = r_imei;
					pMsgToAdmin->admin = r_admin;
					pMsgToAdmin->proposer = r_proposer;

					if (r_watchname != NULL) {
						size_t watchname_len = strlen(r_watchname);
						ntyCopyString(&pMsgToAdmin->watchName, r_watchname, watchname_len);
					}
					if (r_watchimage != NULL) {
						size_t watchimage_len = strlen(r_watchimage);
						ntyCopyString(&pMsgToAdmin->watchImage, r_watchimage, watchimage_len);
					}
					if (r_usercall != NULL) {
						size_t usercall_len = strlen(r_usercall);
						ntyCopyString(&pMsgToAdmin->userName, r_usercall, usercall_len);
					}
					if (r_userimage != NULL) {
						size_t userimage_len = strlen(r_userimage);
						ntyCopyString(&pMsgToAdmin->userImage, r_userimage, userimage_len);
					}

					/*
					pMsgToAdmin->watchName = malloc(watchname_len+1);
					if (pMsgToAdmin->watchName != NULL) {
						memset(pMsgToAdmin->watchName, 0, watchname_len+1);
						memcpy(pMsgToAdmin->watchName, r_watchname, watchname_len);
					}
						
					pMsgToAdmin->watchImage = malloc(watchimage_len+1);
					if (pMsgToAdmin->watchImage != NULL) {
						memset(pMsgToAdmin->watchImage, 0, watchimage_len+1);
						memcpy(pMsgToAdmin->watchImage, r_watchimage, watchimage_len);
					}
					pMsgToAdmin->userName = malloc(usercall_len+1);
					if (pMsgToAdmin->userName != NULL) {
						memset(pMsgToAdmin->userName, 0, usercall_len+1);
						memcpy(pMsgToAdmin->userName, r_usercall, usercall_len);
					}
					pMsgToAdmin->userImage = malloc(userimage_len+1);
					if (pMsgToAdmin->userImage != NULL) {
						memset(pMsgToAdmin->userImage, 0, userimage_len+1);
						memcpy(pMsgToAdmin->userImage, r_userimage, userimage_len);
					}
					*/
						
					ntyVectorInsert(container, pMsgToAdmin, sizeof(BindOfflineMsgToAdmin));	
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_PROPOSER
static int ntyQueryBindOfflineMsgToProposerSelect(void *self, C_DEVID fromId, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ret = -1;
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_BIND_OFFLINE_MSG_TO_PROPOSER, fromId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int bId = ResultSet_getInt(r, 1);

					C_DEVID r_admin = ResultSet_getLLong(r, 2);
					C_DEVID r_imei = ResultSet_getLLong(r, 3);
					const char *r_watchname = ResultSet_getString(r, 4);
					const char *r_watchimage = ResultSet_getString(r, 5);

					BindOfflineMsgToProposer *pMsgToProposer = malloc(sizeof(BindOfflineMsgToProposer));
					if (pMsgToProposer == NULL) {
						ntylog(" %s --> malloc BindOfflineMsgToProposer.details error. \n", __func__);
						break;
					}
					memset(pMsgToProposer, 0, sizeof(BindOfflineMsgToProposer));
										
					//pMsgToProposer->msgId = bId;
					//pMsgToProposer->IMEI = r_imei;
					//pMsgToProposer->admin = r_admin;
					//pMsgToProposer->proposer = r_proposer;
					ntyVectorAdd(container, pMsgToProposer, sizeof(BindOfflineMsgToProposer));
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_DEV_APP_DELETE_FORMAT
static int ntyExecuteDevAppGroupDelete(void *self, C_DEVID aid, C_DEVID did, int *pid) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
#if 0
			Connection_execute(con, NTY_DB_DELETE_GROUP, did, aid);
#endif
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DELETE_GROUP, did, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*pid = ResultSet_getInt(r, 1);
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_LOCATION_INSERT_FORMAT
static int ntyExecuteLocationInsert(void *self, C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info) {	
	ConnectionPool *pool = self;	
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);	
	int ret = 0;	
	TRY
	{		
		con = ntyCheckConnection(self, con);		
		if (con == NULL) {			
			ret = -1;		
		} else {
			U8 sql[256] = {0};
			Connection_execute(con, NTY_DB_NAMES_UTF8_SET_FORMAT);			
			sprintf(sql, NTY_DB_LOCATION_INSERT_FORMAT, did, lng, lat, type, info);			
			ntylog("%s", sql);			
			Connection_execute(con, NTY_DB_LOCATION_INSERT_FORMAT, did, lng, lat, type, info);		
		}	
	}
	CATCH(SQLException)
	{		
		ntylog(" SQLException --> %s\n", Exception_frame.message);		
		ret = -1;	
	}
	FINALLY
	{		
		ntylog(" %s --> Connection_close\n", __func__);		ntyConnectionClose(con);	
	}	
	END_TRY;	
	return ret;
}



//NTY_DB_INSERT_LOCATION
static int ntyExecuteLocationNewInsert(void *self, C_DEVID did, U8 type, const char *lnglat, const char *info, const char *desc) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_INSERT_LOCATION, did, type, lnglat, info, desc);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_LOCATION_INSERT_FORMAT
static int ntyExecuteStepInsert(void *self, C_DEVID did, int value) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_STEP_INSERT_FORMAT, did, value);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_LOCATION_INSERT_FORMAT
int ntyExecuteHeartRateInsert(void *self, C_DEVID did, int value) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_HEARTRATE_INSERT_FORMAT, did, value);
		}	
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DEVICELOGIN_UPDATE_FORMAT
int ntyExecuteDeviceLoginUpdate(void *self, C_DEVID did) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEVICELOGIN_UPDATE_FORMAT, did);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICELOGOUT_UPDATE_FORMAT
int ntyExecuteDeviceLogoutUpdate(void *self, C_DEVID did) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEVICELOGOUT_UPDATE_FORMAT, did);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_PHNUM_VALUE_SELECT_FORMAT
int ntyQueryPhNumSelect(void *self, C_DEVID did, U8 *iccid, U8 *phnum) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	//U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_PHNUM_VALUE_SELECT_FORMAT, did, iccid);

			if (r != NULL) {
				while (ResultSet_next(r)) {
					const char *u8pNum = ResultSet_getString(r, 1);
					int len = strlen(u8pNum);
					ntylog(" ntyQueryPhNumSelect --> len:%d\n", len);
					if (len < 20) {
						memcpy(phnum, u8pNum, len);
					}
				}
			}
			//strcpy(phnum, u8PhNum);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyExecuteDeviceStatusReset(void *self) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DEVICE_STATUS_RESET_FORMAT);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_APPLOGIN_UPDATE_FORMAT
int ntyExecuteAppLoginUpdate(void *self, C_DEVID aid) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_APPLOGIN_UPDATE_FORMAT, aid);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_APPLOGOUT_UPDATE_FORMAT
int ntyExecuteAppLogoutUpdate(void *self, C_DEVID aid) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteAppLogoutUpdate --> execute\n");
			Connection_execute(con, NTY_DB_APPLOGOUT_UPDATE_FORMAT, aid);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_INSERT_EFENCE
int ntyExecuteEfenceInsert(void *self, C_DEVID aid, C_DEVID did, int index, int num, U8 *points, U8 *runtime, int *id) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteEfenceInsert --> executeQuery\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_EFENCE, did, index, num, points, runtime);

			if (r != NULL) {
				if (ResultSet_next(r)) {
					*id = ResultSet_getInt(r, 1);
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_INSERT_FALLDOWN
int ntyExecuteFalldownInsert(void *self, C_DEVID aid, C_DEVID did, double lng, double lat, U8 type, int *id) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteFalldownInsert --> executeQuery\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_FALLDOWN, did, lng, lat, type);

			if (r != NULL) {
				if (ResultSet_next(r)) {
					*id = ResultSet_getInt(r, 1);
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}




//NTY_DB_DELETE_EFENCE
int ntyExecuteEfenceDelete(void *self, C_DEVID aid, C_DEVID did, int index) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyExecuteEfenceDelete --> execute\n");

			U8 sql[512] = {0};		
			sprintf(sql, NTY_DB_DELETE_EFENCE, did, index);
			ntylog("%s", sql);
			Connection_execute(con, NTY_DB_DELETE_EFENCE, did, index);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SELECT_ICCID
int ntyQueryICCIDSelect(void *self, C_DEVID did, const char *iccid, char *phonenum) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog("ntyQueryICCIDSelect --> executeQuery\n");
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_PHNUM_VALUE_SELECT, did, iccid);

			if (r != NULL) {
				if (ResultSet_next(r)) {
					ntylog("ntyQueryICCIDSelect --> ResultSet_getString\n");
					const char *temp = ResultSet_getString(r, 1);
					ntylog("ntyQueryICCIDSelect --> ResultSet_getString : %s\n", temp);
					if (temp != NULL) {
						memcpy(phonenum, temp, strlen(temp));
					}

					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_SET_ICCID
int ntyExecuteICCIDSetInsert(void *self, C_DEVID did, const char *iccid, char *phonenum) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			U8 sql[512] = {0};
			sprintf(sql, NTY_DB_SET_ICCID, did, iccid, phonenum);			
			ntylog("%s\n", sql);	
#if 0
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SET_ICCID, did, iccid, phonenum);
			if (r != NULL) {
				if (ResultSet_next(r)) {
					int tempId = ResultSet_getInt(r, 1);
					if (tempId == 1) {
						ret = 0;
					} else if (tempId == -1) {
						ret = -1;
					}
				}
			}
#else
			Connection_execute(con, NTY_DB_SET_ICCID, did, iccid, phonenum);
			ret = 0;
#endif
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME
int ntyExecuteRuntimeUpdate(void *self, C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME, did, auto_conn, loss_report, light_panel, bell, target_step);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_RUNTIME_AUTOCONN
int ntyExecuteRuntimeAutoConnUpdate(void *self, C_DEVID aid, C_DEVID did, int runtime_param) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_AUTOCONN, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME_LOSSREPORT
int ntyExecuteRuntimeLossReportUpdate(void *self, C_DEVID aid, C_DEVID did, U8 runtime_param) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ntylog(" Connection_execute before ntyExecuteRuntimeLossReportUpdate \n");
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_LOSSREPORT, did, runtime_param);
			ntylog(" Connection_execute after ntyExecuteRuntimeLossReportUpdate \n");
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME_LIGHTPANEL
int ntyExecuteRuntimeLightPanelUpdate(void *self, C_DEVID aid, C_DEVID did, U8 runtime_param) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_LIGHTPANEL, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_RUNTIME_BELL
int ntyExecuteRuntimeBellUpdate(void *self, C_DEVID aid, C_DEVID did, const char *runtime_param) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_BELL, did, runtime_param);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_RUNTIME_TARGETSTEP
int ntyExecuteRuntimeTargetStepUpdate(void *self, C_DEVID aid, C_DEVID did, int runtime_param) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ntylog(" ntyExecuteRuntimeTargetStepUpdate -->  before Connection_execute\n");
			Connection_execute(con, NTY_DB_UPDATE_RUNTIME_TARGETSTEP, did, runtime_param);
			ntylog(" ntyExecuteRuntimeTargetStepUpdate -->  after Connection_execute\n");
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_RUNTIME_MODEL
int ntyExecuteRuntimeModelUpdate( void *self, C_DEVID aid, C_DEVID did, int runtime_param ) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_ERROR;
	Connection_T con = ConnectionPool_getConnection( pool->nPool );
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection( self, con );
		if ( con == NULL ) {
			ret = -1;
		} else {
			
			ntylog(" ntyExecuteRuntimeModelUpdate -->  before Connection_execute\n");
			Connection_execute( con, NTY_DB_UPDATE_RUNTIME_MODEL, did, runtime_param );
			ntylog(" ntyExecuteRuntimeModelUpdate -->  after Connection_execute\n");
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_TURN
int ntyExecuteTurnUpdate(void *self, C_DEVID aid, C_DEVID did, U8 status, const char *ontime, const char *offtime) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ntylog(" ntyExecuteTurnUpdate --> before Connection_execute\n ");
			Connection_execute(con, NTY_DB_UPDATE_TURN, did, status, ontime, offtime);
			ntylog(" ntyExecuteTurnUpdate --> after Connection_execute\n ");
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_PHONEBOOK
int ntyExecuteContactsInsert(void *self, C_DEVID aid, C_DEVID did, Contacts *contacts, int *contactsId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, 
				NTY_DB_INSERT_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone);
			if (r != NULL) {
				if (ResultSet_next(r)) {
					int tempId = ResultSet_getInt(r, 1);
					*contactsId = tempId;
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_PHONEBOOK
int ntyExecuteContactsUpdate(void *self, C_DEVID aid, C_DEVID did, Contacts *contacts, int contactsId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;

	if (contacts == NULL) return NTY_RESULT_FAILED;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_UPDATE_PHONEBOOK, aid,did,contacts->image,contacts->name,contacts->telphone, contactsId);
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DELETE_PHONEBOOK
int ntyExecuteContactsDelete(void *self, C_DEVID aid, C_DEVID did, int contactsId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_DELETE_PHONEBOOK, did, contactsId);
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_SCHEDULE
int ntyExecuteScheduleInsert(void *self, C_DEVID aid, C_DEVID did, const char *daily, const char *time, int status, const char *details, int *scheduleId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_SCHEDULE, did, daily, time, status, details);
			if (r != NULL) {
				if (ResultSet_next(r)) {
					int tempId = ResultSet_getInt(r, 1);
					*scheduleId = tempId;
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DELETE_SCHEDULE
int ntyExecuteScheduleDelete(void *self, C_DEVID aid, C_DEVID did, int scheduleId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			Connection_execute(con, NTY_DB_DELETE_SCHEDULE, did, scheduleId);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_SCHEDULE
int ntyExecuteScheduleUpdate(void *self, C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, int status, const char *details) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteScheduleUpdate --> execute\n");
			Connection_execute(con, NTY_DB_UPDATE_SCHEDULE, did, scheduleId, daily, time, status, details);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_UPDATE_TIMETABLE
int ntyExecuteTimeTablesUpdate(void *self, C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *result) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_UPDATE_TIMETABLE, did, morning, morning_turn, afternoon, afternoon_turn, daily);
			ntylog(" ntyExecuteTimeTablesUpdate ntyExecuteTimeTablesUpdate --> r ::: \n");
			if (r != NULL) {
				while (ResultSet_next(r)) {
					int temp = ResultSet_getInt(r, 1);
					*result = temp;
					ntylog(" ntyExecuteTimeTablesUpdate temp --> r ::: %d\n", temp);
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_INSERT_COMMON_MSG
int ntyExecuteCommonMsgInsert(void *self, C_DEVID sid, C_DEVID gid, char *details, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteCommonMsgInsert --> sid:%lld, gid:%lld, details:%s\n", sid, gid, details);
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_COMMON_MSG, sid, gid, details);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteCommonMsgInsert msgId : %d\n", *msg);
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_LOCATIONREPORT
int ntyExecuteLocationReportInsert(void *self, C_DEVID did, U8 type, const char *info, const char *lnglat, const char *detatils, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteLocationReportInsert --> start\n");
#if 0
			U32 nLength = strlen( info );
			U8 *sqlStr = (U8 *)malloc( nLength + 256 ); //add more 256 bytes.
			if ( sqlStr == NULL ){
				ntylog(" ntyExecuteLocationReportInsert sqlStr malloc failed.\n");
				return NTY_RESULT_FAILED;
			}
			snprintf(sqlStr, nLength+256, NTY_DB_INSERT_LOCATIONREPORT, did, type, info, lnglat, detatils);		
			ntylog("%s", sqlStr);
#endif			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_LOCATIONREPORT, did, type, info, lnglat, detatils);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteHeartReportInsert msgId : %d\n", *msg);
				}
			}
#if 0
			if ( sqlStr != NULL ){
				free(sqlStr);
			}
#endif
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_STEPSREPORT
int ntyExecuteStepsReportInsert(void *self, C_DEVID did, int step, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteStepsReportInsert --> start\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_STEPSREPORT, did, step);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteStepsReportInsert msgId : %d\n", *msg);
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_HEARTREPORT
int ntyExecuteHeartReportInsert(void *self, C_DEVID did, int heart, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteHeartReportInsert --> start\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_HEARTREPORT, did, heart);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteHeartReportInsert msgId : %d\n", *msg);
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_INSERT_BLOODREPORT
int ntyExecuteBloodReportInsert(void *self, C_DEVID did, int blood, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteBloodReportInsert --> start\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_BLOODREPORT, did, blood);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteBloodReportInsert msgId : %d\n", *msg);
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_INSERT_COMMON_ITEM_MSG
int ntyExecuteCommonItemMsgInsert(void *self, C_DEVID sid, C_DEVID gid, char *details, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteCommonItemMsgInsert --> sid:%lld, gid:%lld, details:%s\n", sid, gid, details);
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_COMMON_ITEM_MSG, sid, gid, details);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ntylog("ntyExecuteCommonItemMsgInsert --> getInt\n");
					
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;

					ntylog("ntyExecuteCommonMsgInsert msgId : %d\n", *msg);
				}
			}
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_DELETE_COMMON_ITEM_MSG
int ntyExecuteCommonItemMsgDelete(void *self, int msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteCommonItemMsgDelete --> start\n");
			
			Connection_execute(con, NTY_DB_DELETE_COMMON_ITEM_MSG, msgId);
			ret = 0;
		}
	}
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}



//NTY_DB_INSERT_COMMON_MSG_REJECT
int ntyExecuteCommonMsgToProposerInsert(void *self, C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {

			ntylog(" ntyExecuteCommonMsgToProposerInsert --> executeQuery\n");
			
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_COMMON_MSG_REJECT, sid, gid, detatils);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msg = ResultSet_getInt(r, 1);	
					ret = 0;
				}
				
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


//NTY_DB_UPDATE_DEVICE_STATUS
int ntyExecuteChangeDeviceOnlineStatus(void *self, C_DEVID did, int status) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ntylog(" ntyExecuteChangeDeviceOnlineStatus --> Connection_execute\n");
			Connection_execute(con, NTY_DB_UPDATE_DEVICE_STATUS, did, status);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyQueryDeviceOnlineStatus(void *self, C_DEVID did, int *online) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_DEVICE_STATUS_SELECT_FORMAT, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*online = ResultSet_getInt(r, 1);	
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DEVICE_STATUS_RESET_FORMAT
int ntyQueryAppOnlineStatus(void *self, C_DEVID aid, int *online) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY 
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_APP_STATUS_SELECT_FORMAT, aid);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*online = ResultSet_getInt(r, 1);	
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_INSERT_VOICE_MSG
int ntyQueryVoiceMsgInsert(void *self, C_DEVID senderId, C_DEVID gId, char *filename, int *msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_INSERT_VOICE_MSG, senderId, gId, filename);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*msgId = ResultSet_getInt(r, 1);
					ret = 0;				
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

//NTY_DB_DELETE_VOICE_OFFLINE_MSG
int ntyExecuteVoiceOfflineMsgDelete(void *self, int index, C_DEVID userId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			Connection_execute(con, NTY_DB_DELETE_VOICE_OFFLINE_MSG, index, userId);
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}
//NTY_DB_SELECT_VOICE_MSG
int ntyQueryVoiceMsgSelect(void *self, int index,C_DEVID *senderId, C_DEVID *gId, U8 *filename, long *stamp) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[20] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r =Connection_executeQuery(con, NTY_DB_SELECT_VOICE_MSG, index);

			if (r != NULL) {
				while (ResultSet_next(r)) {
					//*id = ResultSet_getInt(r, 1);	
					*senderId = ResultSet_getLLong(r, 2);
					*gId = ResultSet_getLLong(r, 3);

					const char *details = ResultSet_getString(r, 4);
					int length = strlen(details);

					time_t nStamp = ResultSet_getTimestamp(r, 5);
					*stamp = nStamp;

					memcpy(filename, details, length);

					ntylog(" ntyQueryVoiceMsgSelect --> sender:%lld, gId:%lld\n", *senderId, *gId);
					ntylog(" ntyQueryVoiceMsgSelect --> details : %s\n", details);

					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


int ntyQueryAuthorizeAdmin(void *self, C_DEVID devId, C_DEVID *adminId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = 0;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			ret = -1;

			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_AUTHORIZE_ADMIN, devId);			
			ntylog("%s\n", sql);
			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_AUTHORIZE_ADMIN, devId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					C_DEVID r_admin = ResultSet_getLLong(r, 1);
					*adminId = r_admin;
					ret = 0;
				}
			}
		}
	}
	CATCH(SQLException)
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
	
}


//PROC_INSERT_ADMIN_GROUP
int ntyQueryAdminGroupInsert(void *self, C_DEVID devId, U8 *bname, C_DEVID fromId, U8 *userCall, U8 *wimage, U8 *uimage, U8 *phnum, U32 *msgId) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r =Connection_executeQuery(con, NTY_DB_INSERT_ADMIN_GROUP, devId, bname, fromId, userCall, wimage, uimage);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ret = ResultSet_getInt(r, 1);	
					*msgId = ResultSet_getInt(r, 4);

					const char *pnum = ResultSet_getString(r, 3);
					if (pnum != NULL) {
						memcpy(phnum, pnum, strlen(pnum));
					}
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

int ntyQueryOfflineMsgCounterSelect(void *self, C_DEVID appId, U32 *counter) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ret = -1;
		} else {
			
			ResultSet_T r =Connection_executeQuery(con, NTY_DB_SELECT_OFFLINEMSG_COUNTER, appId);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					*counter = ResultSet_getInt(r, 1);	
					ret = 0;
				}
			}
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}


int ntyExecuteWatchInsertHandle(U8 *imei) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteWatchInsert(pool, imei);
}

int ntyQueryAppIDListSelectHandle(C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAppIDListSelect(pool, did, container);
}


int ntyQueryWatchIDListSelectHandle(C_DEVID aid, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryWatchIDListSelect(pool, aid, container);
}

int ntyQueryDevAppRelationInsertHandle(C_DEVID aid, U8 *imei) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDevAppRelationInsert(pool, aid, imei);
}


int ntyQueryDevAppGroupInsertHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	U8 *Name = "Father";
	return ntyQueryDevAppGroupInsert(pool, aid, did, Name);
}

int ntyQueryDevAppGroupCheckSelectHandle(C_DEVID aid, C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDevAppGroupCheckSelect(pool, aid, did);
}

int ntyExecuteDevAppGroupBindInsertHandle(int msgId, C_DEVID *proposerId, U8 *phonenum, int *pid, char *pname, char *pimage) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppGroupBindInsert(pool, msgId, proposerId, phonenum, pid, pname, pimage);
}

int ntyExecuteDevAppGroupBindAndAgreeInsertHandle(int msgId, C_DEVID *proposerId, char *phonenum, int *pid, char *pname, char *pimage) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppGroupBindAndAgreeInsert(pool, msgId, proposerId, phonenum, pid, pname, pimage);
}

int ntyExecuteBindConfirmDeleteHandle(int msgId, char *phonenum, C_DEVID *id) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteBindConfirmDelete(pool, msgId, phonenum, id);
}

int ntyExecuteCommonOfflineMsgDeleteHandle(int msgId, C_DEVID clientId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonOfflineMsgDelete(pool, msgId, clientId);
}

int ntyQueryCommonOfflineMsgSelectHandle(C_DEVID deviceId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryCommonOfflineMsgSelect(pool, deviceId, container);
}

int ntyQueryVoiceOfflineMsgSelectHandle(C_DEVID fromId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryVoiceOfflineMsgSelect(pool, fromId, container);
}

int ntyQueryPhonebookBindAgreeSelectHandle(C_DEVID did, C_DEVID proposerId, char *phonenum, int *pid, U8 *pname, U8 *pimage) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryPhonebookBindAgreeSelect(pool, did, proposerId, phonenum, pid, pname, pimage);
}

int ntyQueryCommonMsgSelectHandle(C_DEVID msgId, C_DEVID *senderId, C_DEVID *groupId, char *json) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryCommonMsgSelect(pool, msgId, senderId, groupId, json);
}

int ntyQueryBindConfirmInsertHandle(C_DEVID admin, C_DEVID imei, U8 *name, U8 *wimage, C_DEVID proposer, U8 *call, U8 *uimage, int *msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryBindConfirmInsert(pool, admin, imei, name, wimage, proposer, call, uimage, msgId);
}

int ntyQueryPhoneBookSelectHandle(C_DEVID imei, C_DEVID userId, char *phonenum) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryPhoneBookSelect(pool, imei, userId, phonenum);
}

int ntyQueryAdminSelectHandle(C_DEVID did, C_DEVID *appid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAdminSelect(pool, did, appid);
}

int ntyExecuteDevAppGroupDeleteHandle(C_DEVID aid, C_DEVID did, int *pid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDevAppGroupDelete(pool, aid, did, pid);
}

int ntyExecuteLocationInsertHandle(C_DEVID did, U8 *lng, U8 *lat, U8 type, U8 *info) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationInsert(pool, did, lng, lat, type, info);
}

int ntyExecuteLocationNewInsertHandle(C_DEVID did, U8 type, const char *lnglat, const char *info, const char *desc) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationNewInsert(pool, did, type, lnglat, info, desc);
}

int ntyExecuteStepInsertHandle(C_DEVID did, int value) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteStepInsert(pool, did, value);
}

int ntyExecuteHeartRateInsertHandle(C_DEVID did, int value) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteHeartRateInsert(pool, did, value);
}

int ntyExecuteDeviceLoginUpdateHandle(C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDeviceLoginUpdate(pool, did);
}

int ntyExecuteDeviceLogoutUpdateHandle(C_DEVID did) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDeviceLogoutUpdate(pool, did);
}

int ntyExecuteAppLoginUpdateHandle(C_DEVID aid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteAppLoginUpdate(pool, aid);
}

int ntyExecuteAppLogoutUpdateHandle(C_DEVID aid) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteAppLogoutUpdate(pool, aid);
}
int ntyExecuteEfenceInsertHandle(C_DEVID aid, C_DEVID did, int index, int num, U8 *points, U8 *runtime, int *id) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteEfenceInsert(pool, aid, did, index, num, points, runtime, id);
}

int ntyExecuteFalldownInsertHandle(C_DEVID aid, C_DEVID did, double lng, double lat, U8 type, int *id) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteFalldownInsert(pool, aid, did, lng, lat, type, id);
}

int ntyExecuteEfenceDeleteHandle(C_DEVID aid, C_DEVID did, int index) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteEfenceDelete(pool, aid, did, index);
}

int ntyExecuteICCIDSelectHandle(C_DEVID did, const char *iccid, char *phonenum) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryICCIDSelect(pool, did, iccid, phonenum);
}

int ntyExecuteICCIDSetInsertHandle(C_DEVID did, const char *iccid, char *phonenum) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteICCIDSetInsert(pool, did, iccid, phonenum);
}

int ntyExecuteRuntimeUpdateHandle(C_DEVID aid, C_DEVID did, int auto_conn, U8 loss_report, U8 light_panel, const char *bell, int target_step) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeUpdate(pool, aid, did, auto_conn, loss_report, light_panel, bell, target_step);
}

int ntyExecuteRuntimeAutoConnUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeAutoConnUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeLossReportUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeLossReportUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeLightPanelUpdateHandle(C_DEVID aid, C_DEVID did, U8 runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeLightPanelUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeBellUpdateHandle(C_DEVID aid, C_DEVID did, const char *runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeBellUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeTargetStepUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeTargetStepUpdate(pool, aid, did, runtime_param);
}

int ntyExecuteRuntimeModelUpdateHandle(C_DEVID aid, C_DEVID did, int runtime_param) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRuntimeModelUpdate(pool, aid, did, runtime_param);
}


int ntyExecuteTurnUpdateHandle(C_DEVID aid, C_DEVID did, U8 status, const char *ontime, const char *offtime) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteTurnUpdate(pool, aid, did, status, ontime, offtime);
}

int ntyExecuteContactsInsertHandle(C_DEVID aid, C_DEVID did, Contacts *contacts, int *contactsId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteContactsInsert(pool, aid, did, contacts, contactsId);
}

int ntyExecuteContactsUpdateHandle(C_DEVID aid, C_DEVID did, Contacts *contacts, int contactsId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteContactsUpdate(pool, aid, did, contacts, contactsId);
}

int ntyExecuteContactsDeleteHandle(C_DEVID aid, C_DEVID did, int contactsId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteContactsDelete(pool, aid, did, contactsId);
}

int ntyExecuteScheduleInsertHandle(C_DEVID aid, C_DEVID did, const char *daily, const char *time, int status, const char *details, int *scheduleId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleInsert(pool, aid, did, daily, time, status, details, scheduleId);
}

int ntyExecuteScheduleDeleteHandle(C_DEVID aid, C_DEVID did, int scheduleId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleDelete(pool, aid, did, scheduleId);
}

int ntyExecuteScheduleUpdateHandle(C_DEVID aid, C_DEVID did, int scheduleId, const char *daily, const char *time, int status, const char *details) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteScheduleUpdate(pool, aid, did, scheduleId, daily, time, status, details);
}

int ntyExecuteTimeTablesUpdateHandle(C_DEVID aid, C_DEVID did, const char *morning, U8 morning_turn, const char *afternoon,  U8 afternoon_turn, const char *daily, int *result) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteTimeTablesUpdate(pool, aid, did, morning, morning_turn, afternoon, afternoon_turn, daily, result);
}

int ntyExecuteCommonMsgInsertHandle(C_DEVID sid, C_DEVID gid, char *details, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonMsgInsert(pool, sid, gid, details, msg);
}


int ntyExecuteLocationReportInsertHandle(C_DEVID did, U8 type, const char *info, const char *lnglat, const char *detatils, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocationReportInsert(pool, did, type, info, lnglat, detatils, msg);
}

int ntyExecuteStepsReportInsertHandle(C_DEVID did, int step, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteStepsReportInsert(pool, did, step, msg);
}

int ntyExecuteHeartReportInsertHandle(C_DEVID did, int heart, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteHeartReportInsert(pool, did, heart, msg);
}

int ntyExecuteBloodReportInsertHandle(C_DEVID did, int blood, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteBloodReportInsert(pool, did, blood, msg);
}


int ntyExecuteCommonItemMsgInsertHandle(C_DEVID sid, C_DEVID gid, char *details, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonItemMsgInsert(pool, sid, gid, details, msg);
}

int ntyExecuteCommonItemMsgDeleteHandle(int msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonItemMsgDelete(pool, msgId);
}


int ntyQueryVoiceMsgInsertHandle(C_DEVID senderId, C_DEVID gId, char *filename, U32 *msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryVoiceMsgInsert(pool, senderId, gId, filename, msgId);
}

int ntyExecuteVoiceOfflineMsgDeleteHandle(U32 index, C_DEVID userId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteVoiceOfflineMsgDelete(pool, index, userId);
}

int ntyQueryVoiceMsgSelectHandle(U32 index, C_DEVID *senderId, C_DEVID *gId, U8 *filename, long *stamp) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryVoiceMsgSelect(pool, index, senderId, gId, filename, stamp);
}



int ntyQueryPhNumSelectHandle(C_DEVID did, U8 *iccid, U8 *phnum) {
	void *pool = ntyConnectionPoolInstance();
	ntylog(" %s:%lld\n", iccid, did);
	return ntyQueryPhNumSelect(pool, did, iccid, phnum);
}

int ntyExecuteDeviceStatusResetHandle(void) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteDeviceStatusReset(pool);
}

int ntyQueryDeviceOnlineStatusHandle(C_DEVID did, int *online) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryDeviceOnlineStatus(pool, did, online);
}

int ntyQueryAppOnlineStatusHandle(C_DEVID aid, int *online) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAppOnlineStatus(pool, aid, online);
}

int ntyQueryAuthorizeAdminHandle(C_DEVID devId, C_DEVID *adminId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAuthorizeAdmin(pool, devId, adminId);
}

int ntyQueryAdminGroupInsertHandle(C_DEVID devId, U8 *bname, C_DEVID fromId, U8 *userCall, U8 *wimage, U8 *uimage, U8 *phnum, U32 *msgId) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryAdminGroupInsert(pool, devId, bname, fromId, userCall, wimage, uimage, phnum, msgId);
}

int ntyQueryBindOfflineMsgToAdminSelectHandle(C_DEVID fromId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryBindOfflineMsgToAdminSelect(pool, fromId, container);
}

int ntyQueryBindOfflineMsgToProposerSelectHandle(C_DEVID fromId, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryBindOfflineMsgToProposerSelect(pool, fromId, container);
}

int ntyExecuteCommonMsgToProposerInsertHandle(C_DEVID sid, C_DEVID gid, const char *detatils, int *msg) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteCommonMsgToProposerInsert(pool, sid, gid, detatils, msg);
}

int ntyExecuteChangeDeviceOnlineStatusHandle(C_DEVID did, int status) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteChangeDeviceOnlineStatus(pool, did, status);
}

int ntyQueryOfflineMsgCounterSelectHandle(C_DEVID aid, U32 *counter) {
	void *pool = ntyConnectionPoolInstance();
	return ntyQueryOfflineMsgCounterSelect(pool, aid, counter);
}


int ntyConnectionPoolInit(void) {
	//void *pool = ntyConnectionPoolInstance();
	ntyExecuteDeviceStatusResetHandle();
}

void ntyConnectionPoolDeInit(void) {
	void *pool = ntyGetConnectionPoolInstance();
	if (pool != NULL) {
		ntyConnectionPoolRelease(pool);
	}
}

int ntyExecuteClientSelectScheduleHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectSchedule( pool, aid, did, container);
}

int ntyExecuteClientSelectSchedule(void *self, C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if (pool == NULL) return NTY_RESULT_BUSY;
	Connection_T con = ConnectionPool_getConnection(pool->nPool);
	int ret = -1;
	U8 u8PhNum[32] = {0};

	TRY
	{
		con = ntyCheckConnection(self, con);
		if (con == NULL) {
			ntylog("ntyExecuteClientSelectSchedule --> database connection pool is NULL\n");
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_SCHEDULE, did);
			ntylog("ntyExecuteClientSelectSchedule -> sql: %s\n", sql);

			ResultSet_T r = Connection_executeQuery(con, NTY_DB_SELECT_SCHEDULE, did);
			if (r != NULL) {
				while (ResultSet_next(r)) {
					ScheduleSelectItem *objScheduleSelectItem = malloc(sizeof(ScheduleSelectItem));
					if (objScheduleSelectItem == NULL) {
						ntylog(" %s --> malloc objScheduleSelectItem error. \n", __func__);
						break;
					}
					memset(objScheduleSelectItem, 0, sizeof(ScheduleSelectItem));

					const char *r_id = ResultSet_getString(r, 1);
					const char *r_daily = ResultSet_getString(r, 3);
					const char *r_time = ResultSet_getString(r, 4);
					const char *r_details = ResultSet_getString(r, 5);
					const char *r_status = ResultSet_getString(r, 6);

					if (r_id != NULL) {
						size_t len_id = strlen(r_id);
						ntyCopyString(&objScheduleSelectItem->id, r_id, len_id);
					}
					if (r_daily != NULL) {
						size_t len_daily = strlen(r_daily);
						ntyCopyString(&objScheduleSelectItem->daily, r_daily, len_daily);
					}
					if (r_time != NULL) {
						size_t len_time = strlen(r_time);
						ntyCopyString(&objScheduleSelectItem->time, r_time, len_time);
					}
					if (r_details != NULL) {
						size_t len_details = strlen(r_details);
						ntyCopyString(&objScheduleSelectItem->details, r_details, len_details);
					}
					if (r_status != NULL) {
						size_t len_status = strlen(r_status);
						ntyCopyString(&objScheduleSelectItem->status, r_status, len_status);
					}					

					ntyVectorInsert(container, objScheduleSelectItem, sizeof(ScheduleSelectItem));	
				}
			}	
			ret = 0;
		}
	} 
	CATCH(SQLException) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(con);
	}
	END_TRY;

	return ret;
}

int ntyExecuteClientSelectContactsHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectContacts( pool, aid, did, container);
}

int ntyExecuteClientSelectContacts(void *self,C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;

	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectContacts database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_PHONEBOOK, did);
			ntylog("ntyExecuteClientSelectContacts -> sql: %s\n", sql);

			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_PHONEBOOK, did );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {
					ClientContactsAckItem *objClientContactsAckItem = malloc(sizeof(ClientContactsAckItem));
					if (objClientContactsAckItem == NULL) {
						ntylog(" %s --> malloc objClientContactsAckItem error. \n", __func__);
						break;
					}
					memset(objClientContactsAckItem, 0, sizeof(ClientContactsAckItem));

					const char *r_Id = ResultSet_getString(r, 1);
					const char *r_Name = ResultSet_getString(r, 3);
					const char *r_Image = ResultSet_getString(r, 4);
					const char *r_Tel = ResultSet_getString(r, 6);
					const char *r_Admin = ResultSet_getString(r, 7);
					const char *r_App = ResultSet_getString(r, 8);

					if (r_Id != NULL) {
						size_t len_id = strlen(r_Id);
						ntyCopyString(&objClientContactsAckItem->Id, r_Id, len_id);
					}
					if (r_Name != NULL) {
						size_t len_name = strlen(r_Name);
						ntyCopyString(&objClientContactsAckItem->Name, r_Name, len_name);
					}
					if (r_Image != NULL) {
						size_t len_image = strlen(r_Image);
						ntyCopyString(&objClientContactsAckItem->Image, r_Image, len_image);
					}
					if (r_Tel != NULL) {
						size_t len_tel = strlen(r_Tel);
						ntyCopyString(&objClientContactsAckItem->Tel, r_Tel, len_tel);
					}
					if (r_Admin != NULL) {
						size_t len_admin = strlen(r_Admin);
						ntyCopyString(&objClientContactsAckItem->Admin, r_Admin, len_admin);
					}
					if (r_App != NULL) {
						size_t len_app = strlen(r_App);
						ntyCopyString(&objClientContactsAckItem->App, r_App, len_app);
					}
					
					ntyVectorInsert(container, objClientContactsAckItem, sizeof(BindOfflineMsgToAdmin));	
				}
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;
}

int ntyExecuteClientSelectTurnHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectTurn( pool, aid, did, container);
}

int ntyExecuteClientSelectTurn(void *self,C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;
	
	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectTurn database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_SETUP, did);
			ntylog("ntyExecuteClientSelectTurn -> sql: %s\n", sql);
			
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_SETUP, did );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {
					ClientTurnAckItem *objClientTurnAckItem = malloc(sizeof(ClientTurnAckItem));
					if (objClientTurnAckItem == NULL) {
						ntylog(" %s --> malloc objClientTurnAckItem error. \n", __func__);
						break;
					}
					memset(objClientTurnAckItem, 0, sizeof(ClientTurnAckItem));

					const char *r_Status = ResultSet_getString(r, 3);
					const char *r_On = ResultSet_getString(r, 4);
					const char *r_Off = ResultSet_getString(r, 5);

					if (r_Status != NULL) {
						size_t len_Status = strlen(r_Status);
						ntyCopyString(&objClientTurnAckItem->Status, r_Status, len_Status);
					}
					if (r_On != NULL) {
						size_t len_On = strlen(r_On);
						ntyCopyString(&objClientTurnAckItem->On, r_On, len_On);
					}
					if (r_Off != NULL) {
						size_t len_Off = strlen(r_Off);
						ntyCopyString(&objClientTurnAckItem->Off, r_Off, len_Off);
					}

					ntyVectorInsert(container, objClientTurnAckItem, sizeof(ClientTurnAckItem));	
				}							
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;
}


int ntyExecuteClientSelectRunTimeHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectRunTime( pool, aid, did, container);
}

int ntyExecuteClientSelectRunTime( void *self, C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;
	
	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectRunTime database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_RUNTIME, did);
			ntylog("ntyExecuteClientSelectRunTime -> sql: %s\n", sql);

			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_RUNTIME, did );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {	
					ClientRunTimeAckItem *objClientRunTimeAckItem = malloc(sizeof(ClientRunTimeAckItem));
					if (objClientRunTimeAckItem == NULL) {
						ntylog(" %s --> malloc objClientRunTimeAckItem error. \n", __func__);
						break;
					}
					memset(objClientRunTimeAckItem, 0, sizeof(ClientRunTimeAckItem));

					const char *r_AutoConnection = ResultSet_getString(r, 3);
					const char *r_LossReport = ResultSet_getString(r, 4);
					const char *r_LightPanel = ResultSet_getString(r, 5);
					const char *r_WatchBell = ResultSet_getString(r, 6);
					const char *r_TagetStep = ResultSet_getString(r, 7);
					const char *r_Model = ResultSet_getString(r, 8);

					if (r_AutoConnection != NULL) {
						size_t len_autoConnection = strlen(r_AutoConnection);
						ntyCopyString(&objClientRunTimeAckItem->AutoConnection, r_AutoConnection, len_autoConnection);
					}
					if (r_LossReport != NULL) {
						size_t len_lossReport = strlen(r_LossReport);
						ntyCopyString(&objClientRunTimeAckItem->LossReport, r_LossReport, len_lossReport);
					}
					if (r_LightPanel != NULL) {
						size_t len_lightPanel = strlen(r_LightPanel);
						ntyCopyString(&objClientRunTimeAckItem->LightPanel, r_LightPanel, len_lightPanel);
					}
					if (r_WatchBell != NULL) {
						size_t len_watchBell = strlen(r_WatchBell);
						ntyCopyString(&objClientRunTimeAckItem->WatchBell, r_WatchBell, len_watchBell);
					}
					if (r_TagetStep != NULL) {
						size_t len_tagetStep = strlen(r_TagetStep);
						ntyCopyString(&objClientRunTimeAckItem->TagetStep, r_TagetStep, len_tagetStep);
					}
					if (r_Model != NULL) {
						size_t len_Model = strlen(r_Model);
						ntyCopyString(&objClientRunTimeAckItem->Model, r_Model, len_Model);
					}					

					ntyVectorInsert(container, objClientRunTimeAckItem, sizeof(ClientRunTimeAckItem));	
				}
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;
}

int ntyExecuteClientSelectTimeTablesHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectTimeTables( pool, aid, did, container);
}

int ntyExecuteClientSelectTimeTables( void *self, C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;

	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectTimeTables database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_TIMETABLE, did);
			ntylog("ntyExecuteClientSelectTimeTables -> sql: %s\n", sql);
			
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_TIMETABLE, did );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {
					ClientTimeTablesAckItem *objClientTimeTablesAckItem = malloc(sizeof(ClientTimeTablesAckItem));
					if (objClientTimeTablesAckItem == NULL) {
						ntylog(" %s --> malloc objClientTimeTablesAckItem error. \n", __func__);
						break;
					}
					memset(objClientTimeTablesAckItem, 0, sizeof(ClientTimeTablesAckItem));

					const char *r_Morning = ResultSet_getString(r, 3);
					const char *r_MorningTurn = ResultSet_getString(r, 4);
					const char *r_Afternoon = ResultSet_getString(r, 5);
					const char *r_AfternoonTurn = ResultSet_getString(r, 6);
					const char *r_Daily = ResultSet_getString(r, 7);

					if (r_Morning != NULL) {
						size_t len_Morning = strlen(r_Morning);
						ntyCopyString(&objClientTimeTablesAckItem->Morning, r_Morning, len_Morning);
					}
					if (r_MorningTurn != NULL) {
						size_t len_MorningTurn = strlen(r_MorningTurn);
						ntyCopyString(&objClientTimeTablesAckItem->MorningTurn, r_MorningTurn, len_MorningTurn);
					}
					if (r_Afternoon != NULL) {
						size_t len_Afternoon = strlen(r_Afternoon);
						ntyCopyString(&objClientTimeTablesAckItem->Afternoon, r_Afternoon, len_Afternoon);
					}
					if (r_AfternoonTurn != NULL) {
						size_t len_AfternoonTurn = strlen(r_AfternoonTurn);
						ntyCopyString(&objClientTimeTablesAckItem->AfternoonTurn, r_AfternoonTurn, len_AfternoonTurn);
					}
					if (r_Daily != NULL) {
						size_t len_Daily = strlen(r_Daily);
						ntyCopyString(&objClientTimeTablesAckItem->Daily, r_Daily, len_Daily);
					}

					ntyVectorInsert(container, objClientTimeTablesAckItem, sizeof(ClientTimeTablesAckItem));	
				}
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;

}


int ntyExecuteClientSelectLocationHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectLocation( pool, aid, did, container);
}

int ntyExecuteClientSelectLocation( void *self, C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;

	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectLocation database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_LOCATION, did);
			ntylog("ntyExecuteClientSelectLocation -> sql: %s\n", sql);	
	
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_LOCATION, did );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {
					ClientLocationAckResults *pClientLocationAckResults = malloc(sizeof(ClientLocationAckResults));
					if (pClientLocationAckResults == NULL) {
						ntylog(" %s --> malloc faild ClientLocationAckResults. \n", __func__);
						break;
					}
					memset(pClientLocationAckResults, 0, sizeof(ClientLocationAckResults));

					int r_Type = ResultSet_getInt(r, 3);
					const char *r_Radius = ResultSet_getString(r, 4);
					const char *r_Location = ResultSet_getString(r, 5);

					pClientLocationAckResults->Type = r_Type;
					if (r_Radius != NULL) {
						size_t len_Radius = strlen(r_Radius);
						ntyCopyString(&pClientLocationAckResults->Radius, r_Radius, len_Radius);
					}
					if (r_Location != NULL) {
						size_t len_Location = strlen(r_Location);
						ntyCopyString(&pClientLocationAckResults->Location, r_Location, len_Location);
					}

					ntyVectorInsert(container, pClientLocationAckResults, sizeof(ClientLocationAckResults));
				}
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;
}


int ntyExecuteClientSelectEfenceHandle(C_DEVID aid, C_DEVID did, void *container) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectEfence( pool, aid, did, container);
}

int ntyExecuteClientSelectEfence( void *self, C_DEVID aid, C_DEVID did, void *container) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;
	
	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectEfence database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_EFENCE, did);
			ntylog("ntyExecuteClientSelectEfence -> sql: %s\n", sql);
			
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_EFENCE, did );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {
					ClientEfenceListItem *objClientEfenceListItem = malloc(sizeof(ClientEfenceListItem));
					if (objClientEfenceListItem == NULL) {
						ntylog(" %s --> malloc objClientEfenceListItem error. \n", __func__);
						break;
					}
					memset(objClientEfenceListItem, 0, sizeof(ClientEfenceListItem));
					
					const char *r_Index = ResultSet_getString(r, 3);
					const char *r_Num = ResultSet_getString(r, 4);
					const char *r_Points = ResultSet_getString(r, 5);

					if (r_Index != NULL) {
						size_t len_Index = strlen(r_Index);
						ntyCopyString(&objClientEfenceListItem->index, r_Index, len_Index);
					}
					if (r_Num != NULL) {
						size_t len_Num = strlen(r_Num);
						ntyCopyString(&objClientEfenceListItem->num, r_Num, len_Num);
					}
					if (r_Points != NULL) {
						size_t len_Points = strlen(r_Points);
						ntyCopyString(&objClientEfenceListItem->points, r_Points, len_Points);
					}

					ntyVectorInsert(container, objClientEfenceListItem, sizeof(ClientEfenceListItem));
				}
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;
}

int ntyExecuteClientSelectThresholdHandle(C_DEVID did, int type, int *minValue, int *maxValue) {
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteClientSelectThreshold( pool, did, type, minValue, maxValue);
}

int ntyExecuteClientSelectThreshold( void *self, C_DEVID did, int type, int *minValue, int *maxValue) {
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = -1;
	
	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ntylog( "ntyExecuteClientSelectThreshold database connection pool is NULL\n" );
			ret = -1;
		} else {
			U8 sql[256] = {0};		
			sprintf(sql, NTY_DB_SELECT_THRESHOLD, did, type);
			ntylog("ntyExecuteClientSelectThreshold -> sql: %s\n", sql);
			
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_SELECT_THRESHOLD, did, type );
			if ( r != NULL ) {
				while ( ResultSet_next(r) ) {
					*minValue = ResultSet_getInt(r, 2);
					*maxValue = ResultSet_getInt(r, 3);
				}
			}
			ret = 0;
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -2;
	}
	FINALLY
	{
		ntylog(" %s --> Connection_close\n", __func__);
		ntyConnectionClose(conn);
	}
	END_TRY;

	return ret;

}


int ntyExecuteResetHandle( C_DEVID aid, C_DEVID did ){
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteReset( pool, aid, did );

}

int ntyExecuteReset( void *self, C_DEVID aid, C_DEVID did ){
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int ret = 0;

	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ret = -1;
		} else {	
			ntylog(" ntyExecuteReset --> before Connection_execute:CALL PROC_UPDATE_DEVICE_RESET(lld)\n ");
			Connection_executeQuery( conn, NTY_DB_UPDATE_DEVICE_RESET, did );
			ntylog(" ntyExecuteReset --> after Connection_execute:CALL PROC_UPDATE_DEVICE_RESET(lld)\n ");
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog( " %s --> Connection_close\n", __func__ );
		ntyConnectionClose( conn );
	}
	END_TRY;

	return ret;
}

int ntyExecuteRestoreHandle( C_DEVID aid, C_DEVID did ){
	void *pool = ntyConnectionPoolInstance();
	return ntyExecuteRestore( pool, aid, did );
}
int ntyExecuteRestore( void *self, C_DEVID aid, C_DEVID did ){
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool);
	int ret = 0;

	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ) {
			ret = -1;
		} else {	
			ntylog( "ntyExecuteRestore --> before Connection_execute:CALL PROC_UPDATE_DEVICE_CLEAR(%lld)\n ",did );
			Connection_executeQuery( conn, NTY_DB_UPDATE_DEVICE_CLEAR, did );
			ntylog( "ntyExecuteRestore --> after Connection_execute:CALL PROC_UPDATE_DEVICE_CLEAR(%lld)\n ",did );
		}
	} 
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		ret = -1;
	}
	FINALLY
	{
		ntylog( " %s --> Connection_close\n", __func__ );
		ntyConnectionClose( conn );
	}
	END_TRY;

	return ret;

}

int ntyExecuteLocatorBindInsertHandle( C_DEVID appId, C_DEVID devId, LocatorBindReq *pLocatorBindReq ){
	ConnectionPool *pool = ntyConnectionPoolInstance();
	return  ntyExecuteLocatorBindInsert( pool, appId, devId, pLocatorBindReq );
}
/*************************************************************************************************
** Function: ntyExecuteLocatorUnBindDelete 
** Description: locator bind,insert the data from the relation between appid and devid
** Input: void *self:ConnectionPool *pool,C_DEVID appId,C_DEVID devId,LocatorBindReq *pLocatorBindReq
** Output: 
** Return: int 0:succes 5:error:-1,1,2,3,4
** Author:luyb
** Date: 2017-06-02
** Others:
*************************************************************************************************/

int ntyExecuteLocatorBindInsert( void *self, C_DEVID appId, C_DEVID devId, LocatorBindReq *pLocatorBindReq ){
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int nRet = 0;

	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ){
			nRet = -1;
		}else{
			ntylog( "ntyExecuteLocatorBindInsert --> before Connection_execute:CALL PROC_INSERT_GROUP_LOCATOR\n" );
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_INSERT_GROUP_LOCATOR,devId,appId,pLocatorBindReq->Name,pLocatorBindReq->PicURL,pLocatorBindReq->Type );
			if ( r != NULL ){
				while( ResultSet_next(r) ){
					const char *r_result = ResultSet_getString(r, 1);
					if ( r_result != NULL ){
						nRet = atoi( r_result ); //CALL PROC_INSERT_GROUP_LOCATOR return 1,2,3,4,5, and 5 is ok
					}
				}			
			}	
			ntylog( "ntyExecuteLocatorBindInsert --> after Connection_execute:CALL PROC_INSERT_GROUP_LOCATOR nRet=%d\n",nRet );
		}
		
	}
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		nRet = -1;
	}
	FINALLY
	{
		ntylog( " %s --> Connection_close\n", __func__ );
		ntyConnectionClose( conn );
	}
	END_TRY;

	return nRet;

}

int ntyExecuteLocatorUnBindDeleteHandle( C_DEVID appId, C_DEVID devId ){
	ConnectionPool *pool = ntyConnectionPoolInstance();
	return ntyExecuteLocatorUnBindDelete( pool, appId, devId );
}
/*************************************************************************************************
** Function: ntyExecuteLocatorUnBindDelete 
** Description: locator unbind,delete the data from the relation between appid and devid
** Input: void *self:ConnectionPool *pool,C_DEVID appId,C_DEVID devId
** Output: 
** Return: int 0:succes -1:error
** Author:luyb
** Date: 2017-06-02
** Others:
*************************************************************************************************/

int ntyExecuteLocatorUnBindDelete( void *self, C_DEVID appId, C_DEVID devId ){
	ConnectionPool *pool = self;
	if ( pool == NULL ) return NTY_RESULT_BUSY;
	Connection_T conn = ConnectionPool_getConnection( pool->nPool );
	int nRet = 0;
	
	TRY
	{
		conn = ntyCheckConnection( self, conn );
		if ( conn == NULL ){
			nRet = -1;
		}else{
			ntylog( "ntyExecuteLocatorUnBindDelete --> before Connection_execute:CALL PROC_DELETE_GROUP_LOCATOR\n" );
			ResultSet_T r = Connection_executeQuery( conn, NTY_DB_DELETE_GROUP_LOCATOR,devId,appId );
			if ( r != NULL ){
				while( ResultSet_next(r) ){
					const char *r_result = ResultSet_getString(r, 1);
					if ( r_result != NULL ){
						nRet = atoi( r_result );  //CALL PROC_DELETE_GROUP_LOCATOR return 0
					}
				}			
			}	
			ntylog( "ntyExecuteLocatorUnBindDelete --> after Connection_execute:CALL PROC_DELETE_GROUP_LOCATOR nRet=%d\n",nRet );
		}	
	}
	CATCH( SQLException ) 
	{
		ntylog(" SQLException --> %s\n", Exception_frame.message);
		nRet = -1;
	}
	FINALLY
	{
		ntylog( " %s --> Connection_close\n", __func__ );
		ntyConnectionClose( conn );
	}
	END_TRY;

	return nRet;	

}


//end 




#if 0
int main() {
	/*
	C_DEVID proposerId = 0x0;
	int msgId = 17;
	char phonenum[64] = {0};
	ntyExecuteDevAppGroupBindInsertHandle(msgId, &proposerId, phonenum);
	ntylog("phonenum: %s, proposerId:%lld\n", phonenum, proposerId);
	*/

	ntylog("---1--\n");
	char *phonenum1 = NULL;
	size_t len = strlen(phonenum1);
	ntylog("---2- %d-\n", (int)len);

	C_DEVID did = 240207489222799361;
	C_DEVID proposerId = 11780;
	char phonenum[64] = {0};
	strcat(phonenum, "13432992552");
	int pid = 0;
	char *pname = NULL;
	char *pimage = NULL;
	ntylog(" exec ntyQueryPhonebookBindAgreeSelectHandle begin\n");
	ntyQueryPhonebookBindAgreeSelectHandle(did, proposerId, phonenum, &pid, pname, pimage);
	ntylog(" exec ntyQueryPhonebookBindAgreeSelectHandle end\n");
	
}
#endif


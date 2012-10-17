#include "version.h"
#include <string.h>
#include "char.h"
#include "char_base.h"
#include "battle.h"
#include "battle_event.h"
#include "battle_item.h"
#include "battle_magic.h"
#include "item_event.h"
#include "log.h"
#include "anim_tbl.h"
#include "npcutil.h"
#include "magic_base.h"
#include "lssproto_serv.h"


int BATTLE_ItemUseDelete(
	int charaindex,
	int haveitemindex
)
{
	int itemindex;

    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if( ITEM_CHECKINDEX( itemindex ) == FALSE ) return 0;
	{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD �bitem��log���W�[item�W��
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
			"BattleUse(�԰����ϥα����D��)",
	       	CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
        	CHAR_getInt( charaindex,CHAR_Y ),
			ITEM_getChar( itemindex, ITEM_UNIQUECODE),
					ITEM_getChar( itemindex, ITEM_NAME),
					ITEM_getInt( itemindex, ITEM_ID)
		);
	}
	CHAR_DelItemMess( charaindex, haveitemindex, 0);

	return 0;
}

#ifdef _IMPRECATE_ITEM
void ITEM_useImprecate( int charaindex, int toNo, int haveitemindex )
{
	int i;
	int battleindex, attackNo,itemindex;
	char *arg;
	char buf[256];

	struct tagImprecate {
		char fun[256];
		int intfun;
	};
	struct tagImprecate ImList[3] ={
		{"�G",BD_KIND_CURSE},{"��",BD_KIND_BESTOW},{"��",BD_KIND_WISHES} };

	if( !CHAR_CHECKINDEX( charaindex) ) return;
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	if( (attackNo = BATTLE_Index2No( battleindex, charaindex )) < 0 ){
		print( "ANDY attackNo=%d\n", attackNo);
		return;
	}
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	if( arg == NULL ){
		print( "ANDY ITEM id:%d=>arg err\n", ITEM_getInt( itemindex, ITEM_ID));
		return;
	}

	for( i=0; i<3; i++)	{
		if( strstr( arg, ImList[i].fun ) != 0 )	{
			char buf1[256];
			int kind,powers, rounds, HealedEffect;

			if( NPC_Util_GetStrFromStrWithDelim( arg, ImList[i].fun, buf, sizeof( buf)) == NULL )continue;
			kind = ImList[i].intfun;
			if( getStringFromIndexWithDelim( buf,"x", 1, buf1, sizeof( buf1)) == FALSE ) continue;
			powers = atoi( buf1);
			if( getStringFromIndexWithDelim( buf,"x", 2, buf1, sizeof( buf1)) == FALSE ) continue;
			rounds = atoi( buf1);
			HealedEffect = SPR_hoshi;
			BATTLE_ImprecateRecovery(
				battleindex, attackNo, toNo, kind, powers,
				rounds, SPR_item3, HealedEffect );
			CHAR_setItemIndex(charaindex, haveitemindex ,-1);
			CHAR_sendItemDataOne( charaindex, haveitemindex);
			ITEM_endExistItemsOne( itemindex );
			break;
		}
	}
}
#endif

#ifdef _ITEM_MAGICRECOVERY
void ITEM_useMRecovery_Battle( int charaindex, int toNo, int haveitemindex )
{
	int battleindex, attackNo, itemindex;
	int turn=0, magicid, marray;
	char buf1[256];
	char *arg=NULL;

	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	if( BATTLE_CHECKINDEX( battleindex ) == FALSE ) return;
	if( (attackNo =  BATTLE_Index2No( battleindex, charaindex )) == -1 ) return;

    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar( itemindex, ITEM_ARGUMENT );


	if( getStringFromIndexWithDelim( arg, ":", 2, buf1, sizeof(buf1)) ==FALSE ) return;
	turn = atoi( buf1);
	if( getStringFromIndexWithDelim( arg, ":", 1, buf1, sizeof(buf1)) ==FALSE ) return;

	if( strstr( buf1, "��" ) != NULL ){
	}else{
	}

	magicid = ITEM_getInt( itemindex, ITEM_MAGICID);
	marray = MAGIC_getMagicArray( magicid);
	if( marray == -1 ) return;


	if( IsBATTLING( charaindex ) == TRUE ){
		int i, status=-1;
		char *magicarg=NULL, *pszP=NULL;
#ifdef _PREVENT_TEAMATTACK //����u..���o�ϥμĤ�
		int battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
		if( CHAR_getInt( charaindex, CHAR_WHICHTYPE ) == CHAR_TYPEPLAYER
			//&& BattleArray[battleindex].type != BATTLE_TYPE_P_vs_P 
			){
			if( BATTLE_CheckSameSide( charaindex, toNo) == 0 ){//���P��
				int battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
				BATTLE_NoAction( battleindex, BATTLE_Index2No( battleindex, charaindex) );
				CHAR_talkToCli( charaindex, -1, "����u..���o�I���D���a�Ĥ�C", CHAR_COLORYELLOW);
				return;
			}
		}
#endif
		
		magicarg = MAGIC_getChar( marray, MAGIC_OPTION );
		pszP = magicarg;
		for( ;status == -1 && pszP[0] != 0; pszP++ ){
			for( i = 1; i < BATTLE_MD_END; i ++ ){
				if( strncmp( pszP, aszMagicDef[i], 2 ) == 0 ){
					status = i;
					pszP +=2;
					break;
				}
			}
		}
		if( status == -1 ) return;

		BATTLE_MultiMagicDef( battleindex, attackNo, toNo,
			status, turn, MAGIC_EFFECT_USER, SPR_difence );

		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD �bitem��log���W�[item�W��
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
			"BattleUse(�԰����ϥα����D��)",
	       	CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
        	CHAR_getInt( charaindex,CHAR_Y ),
			ITEM_getChar( itemindex, ITEM_UNIQUECODE),
					ITEM_getChar( itemindex, ITEM_NAME),
					ITEM_getInt( itemindex, ITEM_ID)
		);
	}

	CHAR_DelItemMess( charaindex, haveitemindex, 0);
}
#endif

#ifdef _ITEM_USEMAGIC
void ITEM_useMagic_Battle( int charaindex, int toNo, int haveitemindex )
{
	int itemindex,itemmaxuse;
    char *arg=NULL;
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar( itemindex, ITEM_ARGUMENT );

	CHAR_setWorkInt( charaindex, CHAR_WORKBATTLECOM2, toNo );
	CHAR_setWorkInt( charaindex, CHAR_WORKBATTLECOM1, BATTLE_COM_JYUJYUTU );
	CHAR_SETWORKINT_LOW( charaindex, CHAR_WORKBATTLECOM3, atoi(arg) );
	CHAR_SETWORKINT_HIGH( charaindex, CHAR_WORKBATTLECOM3, 0 );
	CHAR_setWorkInt( charaindex, CHAR_WORKBATTLEMODE, BATTLE_CHARMODE_C_OK );

	MAGIC_DirectUse(
				charaindex,
				CHAR_GETWORKINT_LOW( charaindex, CHAR_WORKBATTLECOM3 ),
				CHAR_getWorkInt( charaindex, CHAR_WORKBATTLECOM2 ),
				CHAR_GETWORKINT_HIGH( charaindex, CHAR_WORKBATTLECOM3 )
	);

	itemmaxuse = ITEM_getInt( itemindex, ITEM_DAMAGEBREAK);
	if( itemmaxuse > 0 )
	    ITEM_setInt( itemindex, ITEM_DAMAGEBREAK, itemmaxuse-1 );
    else{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD �bitem��log���W�[item�W��
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
			"BattleUse(�԰����ϥα����D��)",
	       	CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
        	CHAR_getInt( charaindex,CHAR_Y ),
			ITEM_getChar( itemindex, ITEM_UNIQUECODE),
					ITEM_getChar( itemindex, ITEM_NAME),
					ITEM_getInt( itemindex, ITEM_ID)
		);

	    CHAR_DelItemMess( charaindex, haveitemindex, 0);
	}
}
#endif

void ITEM_useRecovery_Battle( int charaindex, int toNo, int haveitemindex )
{
#ifdef _CHANGEITEMUSE	 // Syu ADD �վ�԰����ϥήƲz�]�w
	int power1 = 0;
#endif
	int power = 0, per = 0, HealedEffect=0;
	int battleindex, attackNo,itemindex, kind = BD_KIND_HP;
	char *p = NULL, *arg;
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
#ifdef _CHANGEITEMUSE	 // Syu ADD �վ�԰����ϥήƲz�]�w
	if( ((p = strstr( arg, "��" )) != NULL) && ((p = strstr( arg,"��")) != NULL))
	{
		kind = BD_KIND_HP_MP;
		p = strstr( arg,"��");
		if( sscanf( p+2, "%d", &power1 ) != 1 )
		{
			power1 = 0;
		}
		p = strstr( arg,"��");
	}
	else if( (p = strstr( arg, "��" )) != NULL )
#else
	if( (p = strstr( arg, "��" )) != NULL )
#endif
	{
		kind = BD_KIND_HP;
	}
	else if( (p = strstr( arg, "��" )) != NULL )
	{
		kind = BD_KIND_MP;
	}
	else
#ifdef _ITEM_UNBECOMEPIG
    if( (p = strstr( arg, "�ѽ�" )) != NULL ){
        kind = BD_KIND_UNBECOMEPIG;
		HealedEffect = 100608; //�S�Ľs��
	}
	else
#endif
#ifdef _ITEM_LVUPUP
	if( (p = strstr( arg, "LVUPUP" )) != NULL ){
	    return;
	}
	else
#endif
#ifdef _ITEM_PROPERTY
    if( (p = strstr( arg, "PROPERTY" )) != NULL ){
	    kind = BD_KIND_PROPERTY;
		HealedEffect = 100608; //�S�Ľs��
		if( strstr( arg, "+" ) )
			power = 1;
		else if( strstr( arg, "-" ) )
			power = 2;
	}
	else
#endif
#ifdef _ITEM_ADDPETEXP
    if( (p = strstr( arg, "GETEXP" )) != NULL ){
        return;
	}
	else
#endif
		return;

#ifdef _ITEM_UNBECOMEPIG
    if( HealedEffect != 100608 ){
#endif
	    if( sscanf( p+2, "%d", &power ) != 1 )
		    power = 0;
	    if( power <= 100 )
		    HealedEffect = SPR_heal;//SPR_hoshi
		else if( power <= 300 )
		    HealedEffect = SPR_heal2;
	    else
		    HealedEffect = SPR_heal3;
#ifdef _ITEM_UNBECOMEPIG
	}
#endif

	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	attackNo = BATTLE_Index2No( battleindex, charaindex );
	if( attackNo < 0 )return;

#ifdef _CHANGEITEMUSE	 // Syu ADD �վ�԰����ϥήƲz�]�w
	BATTLE_MultiRecovery( battleindex, attackNo, toNo,
		kind, power, per, SPR_item3, HealedEffect , power1);
#else
	BATTLE_MultiRecovery( battleindex, attackNo, toNo,
		kind, power, per, SPR_item3, HealedEffect );
#endif
	{
		LogItem(
			CHAR_getChar( charaindex, CHAR_NAME ),
			CHAR_getChar( charaindex, CHAR_CDKEY ),
#ifdef _add_item_log_name  // WON ADD �bitem��log���W�[item�W��
			itemindex,
#else
       		ITEM_getInt( itemindex, ITEM_ID ),
#endif
			"BattleUse(�԰����ϥα����D��)",
	       	CHAR_getInt( charaindex,CHAR_FLOOR),
			CHAR_getInt( charaindex,CHAR_X ),
        	CHAR_getInt( charaindex,CHAR_Y ),
			ITEM_getChar( itemindex, ITEM_UNIQUECODE),
					ITEM_getChar( itemindex, ITEM_NAME),
					ITEM_getInt( itemindex, ITEM_ID)
		);
	}
	CHAR_DelItemMess( charaindex, haveitemindex, 0);
}

void ITEM_useStatusChange_Battle(
	int charaindex,
	int toNo,
	int haveitemindex
){
	int turn = 0, i;
	int battleindex, attackNo,itemindex, status = -1, Success = 15;
	int ReceveEffect;
	char *pszP = NULL, *arg;
	char szTurn[] = "turn";
	char szSuccess[] = "��";

    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	pszP = arg;
	for( ;status == -1 && pszP[0] != 0; pszP++ ){
		for( i = 0; i < BATTLE_ST_END; i ++ ){
			if( strncmp( pszP, aszStatus[i], 2 ) == 0 ){
				status = i;
				pszP +=2;
				break;
			}
		}
	}
	if( status == -1 ) return ;
	if( ( pszP = strstr( arg, szTurn ) ) != NULL){
		pszP += sizeof( szTurn );
		sscanf( pszP, "%d", &turn );
	}
	if( ( pszP = strstr( arg, szSuccess ) ) != NULL){
		pszP += sizeof( szSuccess );
		sscanf( pszP, "%d", &Success );
	}

	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	attackNo = BATTLE_Index2No( battleindex, charaindex );

	if( status == BATTLE_ST_NONE ){
		ReceveEffect = SPR_tyusya;
	}else{
		ReceveEffect = SPR_hoshi;
	}
	BATTLE_MultiStatusChange( battleindex, attackNo, toNo,
		status, turn, SPR_item3, ReceveEffect, Success );
	BATTLE_ItemUseDelete( charaindex, haveitemindex );
}

void ITEM_useStatusRecovery_Battle(
	int charaindex, 	// �Ȥä��ͤΥ���ǥå���
	int toNo, 			// �Ȥ���ͤ�  ��
	int haveitemindex 	// �Ȥ��ͤΥ���  ���    ��
){
	int i;
	int battleindex, attackNo,itemindex, status = -1;
	int ReceveEffect;
	char *pszP = NULL, *arg;

	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	// �ѥ�᡼�����  
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	pszP = arg;
	// ���̤��  
	for( ;status == -1 && pszP[0] != 0; pszP++ ){
		// �����⤢��Τǣ����鸡��
		for( i = 0; i < BATTLE_ST_END; i ++ ){
			// ���̥ԥå��꤫��
			if( strncmp( pszP, aszStatus[i], 2 ) == 0 ){
				status = i;
				pszP +=2;
				break;
			}
		}
	}
	// ���̤ʤ��ΤǼ�  
	if( status == -1 ) return ;

	//------- �������������   -----------
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	attackNo = BATTLE_Index2No( battleindex, charaindex );

	ReceveEffect = SPR_tyusya;	//   ��Ȥ��Ϥ���

	// ����
	BATTLE_MultiStatusRecovery( battleindex, attackNo, toNo,
		status, SPR_item3, ReceveEffect );

	// ��    �˥���  �ब�ä������ζ��̽�  
	BATTLE_ItemUseDelete( charaindex, haveitemindex );

}






void ITEM_useMagicDef_Battle(
	int charaindex,
	int toNo,
	int haveitemindex
)
{
	int turn = 0, i;
	int battleindex, attackNo,itemindex, status = -1;
	char *pszP = NULL, *arg;

	char szTurn[] = "turn";

    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	pszP = arg;

	for( ;status == -1 && pszP[0] != 0; pszP++ ){
		for( i = 1; i < BATTLE_MD_END; i ++ ){
			if( strncmp( pszP, aszMagicDef[i], 2 ) == 0 ){
				status = i;
				pszP +=2;
				break;
			}
		}
	}

	if( status == -1 ) return ;

	if( ( pszP = strstr( arg, szTurn ) ) != NULL){
		pszP += sizeof( szTurn );
		sscanf( pszP, "%d", &turn );
	}

	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	attackNo = BATTLE_Index2No( battleindex, charaindex );

	BATTLE_MultiMagicDef( battleindex, attackNo, toNo,
		status, turn, SPR_item3, SPR_difence );

	BATTLE_ItemUseDelete( charaindex, haveitemindex );


}






//--------------------------------------------------------------
//  �ѥ�᡼��  ������  ���Ȥä����ν�  
//--------------------------------------------------------------
// ��    �ξ��
void ITEM_useParamChange_Battle(
	int charaindex, 	// �Ȥä��ͤΥ���ǥå���
	int toNo, 			// �Ȥ���ͤ�  ��
	int haveitemindex 	// �Ȥ��ͤΥ���  ���    ��
)
{
	int i;
	int battleindex, attackNo,itemindex;
	int pow, par = 0;
	int kind = -1;
	char *pszP = NULL, *arg;

	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	// �ѥ�᡼�����  
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	pszP = arg;
	// ���̤��  
	for( ;kind == -1 && pszP[0] != 0; pszP++ ){
		for( i = 1; i < BATTLE_MD_END; i ++ ){
			// ���̥ԥå��꤫��
			if( strncmp( pszP, aszParamChange[i], 2 ) == 0 ){
				kind = i;
				pszP +=2;
				break;
			}
		}
	}
	// ���̤ʤ��ΤǼ�  
	if( kind == -1 ) return ;


	if( strstr( pszP, "%" ) ){	// ���ξ��ϡ�׻�
		par = 1;
	}

	if( sscanf( pszP, "%d", &pow ) != 1 ){
		// ���ݥ���Ȥ����뤫
		pow = 30;
	}

	//------- ���������   -----------
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	attackNo = BATTLE_Index2No( battleindex, charaindex );

	// ����
	BATTLE_MultiParamChange( battleindex, attackNo, toNo,
		kind, pow, par, SPR_item3, SPR_hoshi );


	// ��    �˥���  �ब�ä������ζ��̽�  
	BATTLE_ItemUseDelete( charaindex, haveitemindex );


}






//--------------------------------------------------------------
//  �ե������°��  ������  ���Ȥä����ν�  
//--------------------------------------------------------------
// ��    �ξ��
void ITEM_useFieldChange_Battle(
	int charaindex, 	// �Ȥä��ͤΥ���ǥå���
	int toNo, 			// �Ȥ���ͤ�  ��
	int haveitemindex 	// �Ȥ��ͤΥ���  ���    ��
)
{
	int itemindex;
	char *pArg;

	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	//------- ���������   -----------
	// �ѥ�᡼�����  
	pArg = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	// �ѥ�᡼��  ���ΤǼ�  
	if( pArg == NULL )return ;

	BATTLE_FieldAttChange( charaindex, pArg );

	// ��    �˥���  �ब�ä������ζ��̽�  
	BATTLE_ItemUseDelete( charaindex, haveitemindex );

}


//--------------------------------------------------------------
//  °��  ž����  ���Ȥä����ν�  
//--------------------------------------------------------------
// ��    �ξ��
void ITEM_useAttReverse_Battle(
	int charaindex, 	// �Ȥä��ͤΥ���ǥå���
	int toNo, 			// �Ȥ���ͤ�  ��
	int haveitemindex 	// �Ȥ��ͤΥ���  ���    ��
)
{
	int itemindex, battleindex, attackNo;


	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	//------- ���������   -----------
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	attackNo = BATTLE_Index2No( battleindex, charaindex );

	// ����
	BATTLE_MultiAttReverse( battleindex, attackNo, toNo,
		SPR_item3, SPR_kyu );

	// ��    �˥���  �ब�ä������ζ��̽�  
	BATTLE_ItemUseDelete( charaindex, haveitemindex );


}


//--------------------------------------------------------------
//  ���䤫�������Ȥä����ν�  
//--------------------------------------------------------------
// ��    �ξ��
void ITEM_useRessurect(
	int charaindex, 	// �Ȥä��ͤΥ���ǥå���
	int toNo, 			// �Ȥ���ͤ�  ��
	int haveitemindex 	// �Ȥ��ͤΥ���  ���    ��
)
{
	int itemindex, battleindex, attackNo, par = 0, pow = 0, ReceveEffect;
	char *pszP = NULL;


	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	//------- ���������   -----------
	pszP = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	if( strstr( pszP, "%" ) ){	// ���ξ��ϡ�׻�
		par = 1;
	}

	if( sscanf( pszP, "%d", &pow ) != 1 ){
		// ���ݥ���Ȳ������뤫��
		pow = 0;	// ���ξ��ϴ�������
	}

	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	attackNo = BATTLE_Index2No( battleindex, charaindex );

	if( pow <= 0 ){
		ReceveEffect = SPR_fukkatu3;
	}else
	if( pow <= 100 ){
		ReceveEffect = SPR_fukkatu1;
	}else
	if( pow <= 300 ){
		ReceveEffect = SPR_fukkatu2;
	}else{
		ReceveEffect = SPR_fukkatu3;
	}

	// ����
	BATTLE_MultiRessurect( battleindex, attackNo, toNo,
		pow, par, SPR_item3, ReceveEffect );


	// ��    �˥���  �ब�ä������ζ��̽�  
	BATTLE_ItemUseDelete( charaindex, haveitemindex );

}



//--------------------------------------------------------------
//    ��  ������Ȥä����ν�  
//--------------------------------------------------------------
// ��    �ξ��
void ITEM_useCaptureUp_Battle(
	int charaindex, 	// �Ȥä��ͤΥ���ǥå���
	int toNo, 			// �Ȥ���ͤ�  ��
	int haveitemindex 	// �Ȥ��ͤΥ���  ���    ��
)
{
	int itemindex, battleindex, attackNo, pow = 5, ReceveEffect;
	char *pArg;

	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	//------- ���������   -----------
	// �ѥ�᡼�����  
	pArg = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	// �ѥ�᡼��  ���ΤǼ�  
	if( pArg == NULL )return ;

	if( sscanf( pArg, "%d", &pow ) != 1 ){
		// ���ݥ���Ȳ������뤫��
		pow = 5;
	}

	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );

	attackNo = BATTLE_Index2No( battleindex, charaindex );

	ReceveEffect = SPR_hoshi;

	// ����
	BATTLE_MultiCaptureUp( battleindex, attackNo, toNo,
		pow, SPR_item3, ReceveEffect );

	// ��    �˥���  �ब�ä������ζ��̽�  
	BATTLE_ItemUseDelete( charaindex, haveitemindex );

}
#ifdef _ITEM_CRACKER
void ITEM_useCracker_Effect( charaindex, toindex, haveitemindex)
{
	int itemindex,x,y,tofd;

	itemindex = CHAR_getItemIndex(charaindex,haveitemindex);
	// �ˬd���~
	if(!ITEM_CHECKINDEX(itemindex)) return;

    x = CHAR_getInt( charaindex, CHAR_X);
    y = CHAR_getInt( charaindex, CHAR_Y);
	
	CHAR_setMyPosition( charaindex, x, y, TRUE);
	CHAR_setWorkInt( charaindex, CHAR_WORKITEM_CRACKER, 1);
	CHAR_sendCrackerEffect( charaindex, 101125);
	//to client
	tofd = getfdFromCharaIndex( charaindex );
	lssproto_IC_send(tofd, x, y);
	//�M���D��
	BATTLE_ItemUseDelete(charaindex,haveitemindex);
	CHAR_talkToCli(charaindex,-1,"�D������F�C",CHAR_COLORYELLOW);

}
#endif
#ifdef _ITEM_REFRESH //vincent �Ѱ����`���A�D��
void ITEM_useRefresh_Effect( charaindex, toindex, haveitemindex)
{
	int i,itemindex,ReceveEffect;
	char  *arg;
//	char szBuffer[128]="";
	int status = -1,attackNo,index2;
	int battleindex;
	char *pszP;

//print("\nvincent--ITEM_useRefresh_Effect");
	// �ˬd���~
	itemindex = CHAR_getItemIndex(charaindex,haveitemindex);
	if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );
	pszP = arg;
	// ���̤��  
	for( ;status == -1 && pszP[0] != 0; pszP++ ){
		// �����鸡��
		for( i = 1; i < BATTLE_ST_END; i ++ ){
			// ���̥ԥå��꤫��
			if( strncmp( pszP, aszStatus[i], 2 ) == 0 ){
				status = i;
				pszP +=2;
				break;
			}
		}
	}
//print("\nvincent-->status:%d",status);
	// ���̤ʤ��ΤǼ�  
	if( status == -1 ) return;

	// �Хȥ�  ��
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	index2 = BATTLE_No2Index( battleindex, toindex);//�Q�����ؼФ�index
	attackNo = BATTLE_Index2No( battleindex, charaindex );
//    defNo = BATTLE_Index2No( battleindex, index2 );
//print("\nvincent-->charaindex:%d,attackNo:%d,index2:%d,defNo:%d,",charaindex,attackNo,index2,defNo);

    /* ����饯�����ν�    �ꥹ�Ȥ���õ� */
    CHAR_setItemIndex(charaindex, haveitemindex ,-1);
	CHAR_sendItemDataOne( charaindex, haveitemindex);/* ����  �๹�� */
//	if( CHAR_getWorkInt( charaindex, StatusTbl[status] ) > 0 ){
//		ReceveEffect = SPR_tyusya;
//	}else{
//		ReceveEffect = SPR_hoshi;
//	}
	ReceveEffect = SPR_tyusya;//���w
	BATTLE_MultiStatusRecovery( battleindex, attackNo, toindex,
		status, MAGIC_EFFECT_USER, ReceveEffect );
	/* �ä� */
	ITEM_endExistItemsOne( itemindex );
//////////////////////////
//	BATTLE_MultiList( battleindex, defNo, ToList );
//		 for( i = 0; ToList[i] != -1; i ++ ){
//		toindex = BATTLE_No2Index( battleindex, ToList[i] );
//
//			}

		

////////////////////////////
}
#endif
#ifdef _ITEM_ADDEXP	//vincent �g�紣��
void ITEM_useAddexp_Effect( charaindex, toindex, haveitemindex)
{
	int itemindex,power,vtime;
	//,pindex
	char *p = NULL, *arg;
	char szBuffer[1024]="";

	itemindex = CHAR_getItemIndex(charaindex,haveitemindex);

	// �ˬd���~
	if(!ITEM_CHECKINDEX(itemindex)) return;
	arg = ITEM_getChar(itemindex, ITEM_ARGUMENT );

	if( (p = strstr( arg, "�W" )) != NULL ){
        sscanf( p+2, "%d", &power );
	}
	if( (p = strstr( arg, "��" )) != NULL ){
		sscanf( p+2, "%d", &vtime );
	}
	if( p == NULL )return;
#ifdef _ITEM_ADDEXP2 // ���G�ɶ��i�H�֥[
	{
		int point;
		point = CHAR_getWorkInt( charaindex, CHAR_WORKITEM_ADDEXP)
					* (int)(CHAR_getWorkInt( charaindex, CHAR_WORKITEM_ADDEXPTIME)/60);
		if( point >= 72000) {
			CHAR_talkToCli( charaindex, -1, "���z���G�֭p�ɶ��ήĪG�w�F�W���C", CHAR_COLORYELLOW);
			return;
		}
		point += (power * vtime);
		point = min( point, 72000);
		vtime = (int)(point / power);
#ifdef _LOG_OTHER
		sprintf( szBuffer, "�ϥδ��z���G %d\t�ֿn�ĪG= ����%d ��O%d�H �ɶ�%d�� ",
				ITEM_getInt( itemindex, ITEM_ID ), point, power, vtime);
		LogOther( 
			CHAR_getChar( charaindex, CHAR_CDKEY),
			CHAR_getChar( charaindex, CHAR_NAME),
			szBuffer );
#endif
		//CHAR_setInt( charaindex, CHAR_ADDEXPPOWER, power);
		//CHAR_setInt( charaindex, CHAR_ADDEXPTIME,vtime*60 );

		//sprintf(szBuffer, "���հT���G�ثe����%d �ɶ�%d���C", point, vtime*60);
		//CHAR_talkToCli(charaindex,-1,szBuffer,CHAR_COLORRED);
	}
#endif
	CHAR_setWorkInt( charaindex, CHAR_WORKITEM_ADDEXP, power);
	CHAR_setWorkInt( charaindex, CHAR_WORKITEM_ADDEXPTIME,vtime*60 );

	//�M���D��
	BATTLE_ItemUseDelete(charaindex,haveitemindex);
	//sprintf(szBuffer, "�ǲ߸g�窺��O���ɤF%d�H", power);
	sprintf(szBuffer, "�ǲ߸g�窺��O���ɤF%d�H�A�ɮĳѾl%d�����C", power, vtime);
	CHAR_talkToCli(charaindex,-1,szBuffer,CHAR_COLORYELLOW);

}
#endif
//Terry add 2001/12/24
#ifdef _ITEM_FIRECRACKER
void ITEM_useFirecracker_Battle( charaindex, toindex, haveitemindex)
{
	int itemindex, battleindex, masteridx=-1, index2;
	
	char szWork[128];
#ifdef _FIX_FIRECRACKER
	int petid=-1, i=1;
	BOOL FINDPET=FALSE;
	char *buff1;
	char token[256], buf1[256];
#else
	int PetEscape = 0;
#endif

	itemindex = CHAR_getItemIndex(charaindex,haveitemindex);
	// �ˬd���~
	if(!ITEM_CHECKINDEX(itemindex)) return;

	battleindex = CHAR_getWorkInt(charaindex,CHAR_WORKBATTLEINDEX);
#ifdef _FIX_FIRECRACKER
	index2 = BATTLE_No2Index( battleindex, toindex);
	if( !CHAR_CHECKINDEX( index2) ) return;
	{
		int attackNo = BATTLE_Index2No( battleindex, charaindex );
		int safeSide = 0;
		int MySide_start, MySide_end;
		if( attackNo >= 10 )
			safeSide = 1;

		MySide_start = safeSide*SIDE_OFFSET;
		MySide_end = ((safeSide*SIDE_OFFSET) + SIDE_OFFSET);

		if( (toindex >= MySide_start) && (toindex<MySide_end) ){	//�P��
			return;
		}
	}

	if( BATTLE_Index2No( battleindex, charaindex ) == toindex ){
		BATTLE_ItemUseDelete(charaindex,haveitemindex);//�Φb�ۨ�
		return;
	}

	if( CHAR_getInt( index2, CHAR_WHICHTYPE) == CHAR_TYPEPLAYER ) {
		petid = -1;//�@�Φb�H��
	}else if( CHAR_getInt( index2, CHAR_WHICHTYPE) == CHAR_TYPEPET ) {
		masteridx = BATTLE_No2Index(battleindex,toindex-5);//�@�Φb�d��

		if( !CHAR_CHECKINDEX( masteridx) ) return;

		petid = CHAR_getInt(index2,CHAR_PETID);//�d��id
	}else if( CHAR_getInt( index2, CHAR_WHICHTYPE) == CHAR_TYPEENEMY ) {
		petid = CHAR_getInt(index2,CHAR_PETID);
	}

	if( (buff1 = ITEM_getChar( itemindex, ITEM_ARGUMENT)) == NULL ) return;

	memset( token, 0, sizeof( token));
	if( NPC_Util_GetStrFromStrWithDelim( buff1, "KPET", token, sizeof( token) ) == NULL) {
		print( "Can't get KPET: %s!!\n", buff1);
		return;
	}

	// won fix
	while( getStringFromIndexWithDelim( token,"_", i, buf1, sizeof( buf1)) != FALSE )	{
	//while( getStringFromIndexWithDelim( token,"|", i, buf1, sizeof( buf1)) != FALSE )	{
		i++;
		if( petid == atoi( buf1) ){//�~�~�P�O
			FINDPET = TRUE;
			break;
		}
	}

	sprintf(szWork,"BB|a%X|w3|r%X|f0|d0|p0|FF|",BATTLE_Index2No(battleindex,charaindex),toindex);
	BATTLESTR_ADD(szWork);//��X�@�����ʵe
//�e�T���ܪ��a
	if( FINDPET == TRUE )	{//�Y���~�~
		char buf4[255];
		sprintf( buf4, "%s�Q�~�]�F�I", CHAR_getChar( index2, CHAR_NAME));

		BATTLE_Exit(index2,battleindex);//���}�԰�
		if( CHAR_CHECKINDEX( masteridx) ){
			CHAR_setInt(masteridx,CHAR_DEFAULTPET,-1);//�L�Ѿ��d
			CHAR_talkToCli( masteridx,-1, buf4, CHAR_COLORYELLOW);
		}

		sprintf(szWork,"BQ|e%X|",toindex);//�k�]�ʵe
		BATTLESTR_ADD(szWork);
		CHAR_talkToCli( charaindex,-1, buf4, CHAR_COLORYELLOW);
	}else	{
		CHAR_talkToCli( charaindex, -1, "����Ƴ��S�o�͡C", CHAR_COLORYELLOW);
	}
	BATTLE_ItemUseDelete( charaindex, haveitemindex);//�R���@��
#else

	index2 = BATTLE_No2Index(battleindex,toindex);
	// �Q���쪺�O�_���d��
	if(CHAR_getInt(index2,CHAR_WHICHTYPE) == CHAR_TYPEPET) {
		// �p�G�Q�����O�~�~
		if(CHAR_getInt(index2,CHAR_PETID) >= 901 && CHAR_getInt(index2,CHAR_PETID) <= 904){
			// ���o�ؼЪ�����誺���⪺index
			masteridx = BATTLE_No2Index(battleindex,toindex-5);
			// �p�G��Ԫ��O���a
			if(CHAR_getInt(masteridx,CHAR_WHICHTYPE) == CHAR_TYPEPLAYER) PetEscape = 1;
			else																									       print("\n�~�~�b�J�ĮɥX�{");
		}
	}
	
	// �p�G���O���ۤv
	if(BATTLE_Index2No(battleindex,charaindex) != toindex)
	{
		sprintf(szWork,"BB|a%X|w3|r%X|f0|d0|p0|FF|",BATTLE_Index2No(battleindex,charaindex),toindex);
		BATTLESTR_ADD(szWork);
		if(PetEscape)
		{
			BATTLE_Exit(index2,battleindex);
			CHAR_setInt(masteridx,CHAR_DEFAULTPET,-1);
			sprintf(szWork,"BQ|e%X|",toindex);
			BATTLESTR_ADD(szWork);
			CHAR_talkToCli(masteridx,-1,"�~�~�Q�~�]�F�I",CHAR_COLORWHITE);
			CHAR_talkToCli(charaindex,-1,"�~�~�Q�~�]�F�I",CHAR_COLORWHITE);
		}
	}
	CHAR_talkToCli(charaindex,-1,"����Ƴ��S�o�͡C",CHAR_COLORWHITE);
	CHAR_talkToCli(charaindex,-1,"�D������F�C",CHAR_COLORWHITE);
	// �ϥΫ�D�����
	BATTLE_ItemUseDelete(charaindex,haveitemindex);
#endif
}
#endif

//Terry end




//�𵴪��A�^�_�@�O,�_�[�_��(��,��,�u)���F �ѼƦP�]�k(��,��,�u)���F,���u��b�԰����ϥ�,�Χ��N�S�F 
//�Ѽ� �Ҧp:�@�O�� �l turn 1
#ifdef  _FEV_ADD_NEW_ITEM			// FEV ADD �W�[�_���u��
void ITEM_ResAndDef( int charaindex, int toindex, int haveitemindex )
{
	int itemindex, battleindex, attackNo, par = 0, pow = 0, ReceveEffect;
	char *buffer = NULL;
	char *magicarg = NULL;
	char *magicarg2 = NULL;
	char *magicarg3 = NULL;
	char magicarg4[200];

    char *pszP = NULL; 
	char delim[] = " ";//��U���Ѧr�ꪺ�Ϲj�r��

	int status = -1, i, turn = 3;
	char szTurn[] = "turn";

	// ����  �ब���뤫�ɤ���
    itemindex = CHAR_getItemIndex( charaindex, haveitemindex);
    if(!ITEM_CHECKINDEX(itemindex)) return;

	buffer = ITEM_getChar(itemindex, ITEM_ARGUMENT );//���r��
	pszP = strtok(buffer, delim);//�Ĥ@�ӰѼ�
    magicarg = strtok(NULL, delim);//�ĤG�ӰѼ�
	magicarg2 = strtok(NULL, delim);//�ĤT�ӰѼ�
	magicarg3 = strtok(NULL, delim);//�ĥ|�ӰѼ�
    sprintf(magicarg4,"%s %s %s",magicarg,magicarg2,magicarg3);
	magicarg = (char*)magicarg4;

	//����u..���o�ϥμĤ�
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	if( CHAR_getInt( charaindex, CHAR_WHICHTYPE ) == CHAR_TYPEPLAYER
		//&& BattleArray[battleindex].type != BATTLE_TYPE_P_vs_P 
		){
		if( BATTLE_CheckSameSide( charaindex, toindex) == 0 ){//���P��
			battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
			BATTLE_NoAction( battleindex, BATTLE_Index2No( battleindex, charaindex) );
			CHAR_talkToCli( charaindex, -1, "����u..���o�I���D���a�Ĥ�C", CHAR_COLORYELLOW);
			return;
		}
	}

	//�b�𵴪��A�^�_�@�O
	if( strstr( pszP, "%" ) ){
		par = 1;
	}
	
	if( sscanf( pszP, "%d", &pow ) != 1 ){
		pow = 0;
	}
      
	attackNo = BATTLE_Index2No( battleindex, charaindex );
   
	//��ܥN�����S��
	if( pow <= 0 ){
		ReceveEffect = SPR_fukkatu3;
	}else
	if( pow <= 100 ){
		ReceveEffect = SPR_fukkatu1;
	}else
	if( pow <= 300 ){
		ReceveEffect = SPR_fukkatu2;
	}else{
		ReceveEffect = SPR_fukkatu3;
	}

	BATTLE_MultiRessurect( battleindex, attackNo, toindex,
		pow, par, SPR_item3, ReceveEffect );
    

	//(��,��,�u)���F

	// ���̤��  
	for( ;status == -1 && magicarg[0] != 0; magicarg++ ){
		for( i = 1; i < BATTLE_MD_END; i ++ ){
			// ���̥ԥå��꤫��
			if( strncmp( magicarg, aszMagicDef[i], 2 ) == 0 ){
				status = i;
				pszP +=2;
				break;
			}
		}
	}
	// ���̤ʤ��ΤǼ�  
	if( status == -1 ) return;

	// ����  �������뤫��
	if( ( magicarg = strstr( magicarg, szTurn ) ) != NULL){
		magicarg += sizeof( szTurn );
		sscanf( magicarg, "%d", &turn );
	}

	//print("����:%d",turn);

	// �Хȥ�  ��
	battleindex = CHAR_getWorkInt( charaindex, CHAR_WORKBATTLEINDEX );
	attackNo =  BATTLE_Index2No( battleindex, charaindex );

	// ����
	BATTLE_MultiMagicDef( battleindex, attackNo, toindex,
		status, turn, MAGIC_EFFECT_USER, SPR_difence );


	BATTLE_ItemUseDelete( charaindex, haveitemindex );

}

#endif 





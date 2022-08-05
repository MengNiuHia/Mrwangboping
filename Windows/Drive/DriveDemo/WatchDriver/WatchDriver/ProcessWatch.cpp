#include "ProcessWatch.hpp"

extern PDEVICE_OBJECT g_pDeviceObject;

//���̼��ӻص�����
VOID ProcessMonitorCallback(
	IN HANDLE hParentId,
	IN HANDLE hProcessId,
	IN BOOLEAN bCreate)
{
	//�õ��豸��չ�Ľṹָ��
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)g_pDeviceObject->DeviceExtension;
	//�ȱ�������չ����ǰ���豸��չ�ṹ������Ӧ�ò�
	pDevExt->hPParentId = hParentId;
	pDevExt->hPProcessId = hProcessId;
	pDevExt->bPCreate = bCreate;

	if (bCreate)
		DbgPrint("�������� ����ID =  = %d\n", hProcessId);
	else
		DbgPrint("�˳����� ����ID =  = %d\n", hProcessId);
	
	//������֪ͨӦ�ò�������򣬵����������� �Ƿ�ȴ������TRUE,��Ҫ��������kewait
	// ����¼���֪ͨ�¼�����ϵͳ�᳢�������¼������Ͼ����ܶ�ĵȴ��� �ڶ� KeClearEvent �� KeResetEvent �ĵ������֮ǰ���¼��������źš� ����¼���ͬ���¼�����ϵͳ�Զ�����¼�֮ǰ�����ȵȴ�һ�Ρ�
	KeSetEvent(pDevExt->pEvent, 0, FALSE);
	//����Ϊ������״̬
	KeClearEvent(pDevExt->pEvent);

}
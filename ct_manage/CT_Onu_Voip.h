
enum
{
   voice_global_config_code_min= 0,
   voice_global_config_ip_mode,
   voice_global_config_ip_addr,
   voice_global_config_ip_gw,
   voice_global_config_pppoe_mode,
   voice_global_config_pppoe_username,
   voice_global_config_pppoe_password,
   voice_global_config_tagged_mode,
   voice_global_config_vlan,
   voice_global_config_priority,
   voice_global_config_code_max,
};
enum
{
    h248_config_code_min = 0,
    h248_config_local_port,
    h248_config_mgc_addr,
    h248_config_mgc_port,
    h248_config_mgc_backup_addr,
    h248_config_mgc_backup_port,
    h248_config_regiser_mode,
    h248_config_local_domain,
    h248_config_heartbeat_enable,
    h248_config_heartbeat_time,
    h248_config_code_max     
};
enum
{
    sip_config_code_min = 0,    
    sip_config_local_port,
    sip_config_server_ip,
    sip_config_server_port,
    sip_config_server_backup_ip,
    sip_config_server_backup_port,
    sip_config_reg_server_ip,
    sip_config_reg_server_port,
    sip_config_reg_server_backup_ip,
    sip_config_reg_server_backup_port,
    sip_config_outbound_server_ip,
    sip_config_outbound_server_port,
    sip_config_reg_interval,
    sip_config_heartbeat_enable,
    sip_config_heartbeat_time,
    sip_config_code_max
};
enum
{
    sip_user_config_code_min,    
    sip_user_config_account,    
    sip_user_config_username,    
    sip_user_config_password,    
    sip_user_config_code_max    
};
enum
{
    voice_fax_config_code_min,
    voice_fax_config_t38_enable,
    voice_fax_config_control,
    voice_fax_config_code_max    
};

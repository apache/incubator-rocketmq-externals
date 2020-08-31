package org.apache.rocketmq.connect.es.config;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

import org.apache.rocketmq.connect.es.model.ModelType;
import org.elasticsearch.client.RestHighLevelClient;

public class MapperConfig {

	/**
	 * mapper名
	 */
	private String mapperName;
	
	
	/**
	 * 表名
	 */
	private String tableName;
	
	
	/**
	 * 客服端名字，如果没有就默认
	 */
	private String clientName;
	
	private RestHighLevelClient restHighLevelClient;
	
	/**
	 * 字段名映射方式
	 */
	private NamingMethod  namingMethod;
	
	/**
	 * 数据库字段与key的映射关系
	 */
	private Map<String ,String> fieldAndKeyMapper;
		
	/**
	 * 映射模式，简单，一方，多方
	 */
	private ModelType mapperType;

	
	/**
	 * id前缀
	 */
	private String idPrefix;
	
	/**
	 * 唯一建名
	 */
	private String uniqueName;
	
	/**
	 * id前缀
	 */
	private String mainIdPrefix;
	
	/**
	 * 唯一建名
	 */
	private String mainRelationField;
	
	/**
	 * 
	 */
	private String fromRelationField;
	
	/**
	 * es index
	 */
	private String index;
	
	/**
	 * es type 为了兼容7.0以下版本的es
	 */
	private String type;
	
	/**
	 * 逻辑删除的字段
	 */
	private String logicDeleteFieldName;
	
	/**
	 * 逻辑删除的值
	 */
	private String logicDeleteFieldValue;
	
	private boolean updateContrast;
	
	/**
	 * 一对多，一的一方，一对一，从方
	 */
	private List<MapperConfig> oneWaysMapperConfig = new ArrayList<>();
	
	/**
	 * 一对多，多方
	 */
	private List<MapperConfig> manyWaysMapperConfig = new ArrayList<>();
	
	/**
	 * 主数据
	 */
	private List<RelationConfig> relationConfigList = new ArrayList<>();
	
	/**
	 * 分表名
	 */
	private List<String> subTableName = new CopyOnWriteArrayList<String>();

	public String getMapperName() {
		return mapperName;
	}


	public void setMapperName(String mapperName) {
		this.mapperName = mapperName;
	}


	public String getTableName() {
		return tableName;
	}


	public void setTableName(String tableName) {
		this.tableName = tableName;
	}


	public String getClientName() {
		return clientName;
	}


	public void setClientName(String clientName) {
		this.clientName = clientName;
	}

	public RestHighLevelClient getRestHighLevelClient() {
		return restHighLevelClient;
	}


	public void setRestHighLevelClient(RestHighLevelClient restHighLevelClient) {
		this.restHighLevelClient = restHighLevelClient;
	}


	public NamingMethod getNamingMethod() {
		return namingMethod;
	}


	public void setNamingMethod(NamingMethod namingMethod) {
		this.namingMethod = namingMethod;
	}


	public Map<String, String> getFieldAndKeyMapper() {
		return fieldAndKeyMapper;
	}


	public void setFieldAndKeyMapper(Map<String, String> mapper) {
		this.fieldAndKeyMapper = mapper;
	}


	public ModelType getMapperType() {
		return mapperType;
	}


	public void setMapperType(ModelType mapperType) {
		this.mapperType = mapperType;
	}

	public String getIdPrefix() {
		return idPrefix;
	}


	public void setIdPrefix(String idPrefix) {
		this.idPrefix = idPrefix;
	}


	public String getUniqueName() {
		return uniqueName;
	}


	public void setUniqueName(String uniqueName) {
		this.uniqueName = uniqueName;
	}


	public String getIndex() {
		return index;
	}


	public void setIndex(String index) {
		this.index = index;
	}

	public String getType() {
		return type;
	}


	public void setType(String type) {
		this.type = type;
	}


	public String getMainIdPrefix() {
		return mainIdPrefix;
	}


	public void setMainIdPrefix(String mainIdPrefix) {
		this.mainIdPrefix = mainIdPrefix;
	}


	


	public String getMainRelationField() {
		return mainRelationField;
	}


	public void setMainRelationField(String mainRelationField) {
		this.mainRelationField = mainRelationField;
	}

	public String getFromRelationField() {
		return fromRelationField;
	}


	public void setFromRelationField(String fromRelationField) {
		this.fromRelationField = fromRelationField;
	}


	public String getLogicDeleteFieldName() {
		return logicDeleteFieldName;
	}


	public void setLogicDeleteFieldName(String logicDeleteFieldName) {
		this.logicDeleteFieldName = logicDeleteFieldName;
	}


	public String getLogicDeleteFieldValue() {
		return logicDeleteFieldValue;
	}


	public void setLogicDeleteFieldValue(String logicDeleteFieldValue) {
		this.logicDeleteFieldValue = logicDeleteFieldValue;
	}


	public boolean isUpdateContrast() {
		return updateContrast;
	}


	public void setUpdateContrast(boolean updateContrast) {
		this.updateContrast = updateContrast;
	}


	public List<MapperConfig> getOneWaysMapperConfig() {
		return oneWaysMapperConfig;
	}


	public void setOneWaysMapperConfig(List<MapperConfig> oneWaysapperConfig) {
		this.oneWaysMapperConfig = oneWaysapperConfig;
	}


	public List<MapperConfig> getManyWaysMapperConfig() {
		return manyWaysMapperConfig;
	}


	public void setManyWaysMapperConfig(List<MapperConfig> manyWaysMapperConfig) {
		this.manyWaysMapperConfig = manyWaysMapperConfig;
	}

	public List<RelationConfig> getRelationConfigList() {
		return relationConfigList;
	}


	public void addRelationConfig(RelationConfig relationConfig) {
		this.relationConfigList.add(relationConfig);
	}

	
	public void setRelationConfigList(List<RelationConfig> relationConfigList) {
		this.relationConfigList = relationConfigList;
	}


	public List<String> getSubTableName() {
		return subTableName;
	}


	public void setSubTableName(List<String> subTableName) {
		this.subTableName = subTableName;
	}

	
	

}

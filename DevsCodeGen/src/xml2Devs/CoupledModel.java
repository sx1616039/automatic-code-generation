package xml2Devs;

import org.dom4j.Element;
import org.dom4j.QName;
import xml2Devs.structs.ClassAttribute;
import xml2Devs.structs.ClassFunction;
import xml2Devs.structs.Connector;

import java.io.*;
import java.util.*;

import static xmlTools.GeneralParseXMLMethod.findAllChildElementByAttributeText;
import static xmlTools.GeneralParseXMLMethod.findChildElementByAttribute;

public class CoupledModel {
    private String className = "";
    private String head_file = "";
    private String cpp_file = "";
    private String constructor = "";
    private String destructor = "";
    List<ClassFunction> self_defined_funcs;
    List<ClassAttribute> attrs;
    List<Connector> connectors;
    Element model;
    GlobalElements globalElements;
    public CoupledModel(Element model, GlobalElements globalElements) throws IOException {
        this.model = model;
        this.globalElements = globalElements;
        attrs = new ArrayList<>();
        self_defined_funcs = new ArrayList<>();
        connectors = new ArrayList<>();
        className = model.attributeValue("name");
        findAttributes();
        findFunctions();
        findConnector();
        add_head_file();
        add_cpp_file();
        writeToFile();

    }
    public void add_head_file() {
        head_file += "#pragma once\n";
        head_file += "#ifndef "+className.toUpperCase()+"_H\n";
        head_file += "#define "+className.toUpperCase()+"_H\n";
        head_file += "#include \"ExchangeData.h\"\n";
        head_file += "#include \"Vector.h\"\n";
        for (ClassAttribute attr:attrs){
            List<Element> classElementList = globalElements.getClassElementList();
            for (Element tmp:classElementList){// 加载原子模型组件头文件
                if (tmp.attributeValue("name").equals(attr.type)){
                    head_file += "#include \"" + attr.type+".h\"\n";
                    break;
                }
            }
        }
        head_file += "#include <iostream>\n";
        head_file += "using namespace std;\n";
        head_file += "using namespace adevs;\n";
        head_file += "class "+className+" : public adevs::Digraph<ExchangeData*> {\n";
        head_file += "public:\n\t";
        head_file += className+"(";
        for (ClassFunction func:self_defined_funcs){
            if (func.name.equals(className)){
                for (ClassAttribute attr:func.paraList){
                    head_file += attr.type+" "+attr.name+",";
                }
                head_file = head_file.substring(0,head_file.lastIndexOf(","));
            }
        }
        head_file += ");\n";
        head_file += "\t~"+className+"();\n";
        head_file += "public:\n\t";
        for (ClassAttribute attr:attrs){
            if (attr.isStatic!=null && attr.isStatic.equals("true")){
                head_file += "static ";
            }
            List<Element> classElementList = globalElements.getClassElementList();
            boolean isClass = false;
            for (Element tmp:classElementList){// 原子模型组件
                if (tmp.attributeValue("name").equals(attr.type)){
                    if (isContinuousModel(globalElements.getRoot(),tmp)){
                        head_file += "Hybrid<IO_Type> *m_hybrid_" +attr.name +";\n\t";
                    }
                    head_file += attr.type+" *"+attr.name;
                    isClass = true;
                    break;
                }
            }
            if (!isClass){
                head_file += attr.type+" "+attr.name;
            }

            if (attr.multiplicity!=null && !attr.multiplicity.equals("")){
                head_file += "[" + attr.multiplicity + "]";
            }
            head_file += ";\n\t";
        }
        head_file += "\n";
        head_file += "public:\n\t";
        for (ClassFunction func:self_defined_funcs){
            if (func.returnType!=null && !func.returnType.equals("")){
                head_file += func.returnType + " " + func.name + "(";
                if (func.paraList.size()>0){
                    for (ClassAttribute attr:func.paraList){
                        head_file += attr.type+" "+attr.name+",";
                    }
                    head_file = head_file.substring(0,head_file.lastIndexOf(","));
                }
                head_file += ");\n";
            }
        }
        head_file += "\n};\n";
        head_file += "#endif\n";
//        System.out.println(head_file);

    }
    public void add_cpp_file() {
        cpp_file += "#include \"" + className + ".h\"\n";
        int i = 0;
        for (ClassAttribute attr : attrs) {// 静态变量初始化
            if (attr.isStatic != null && attr.isStatic.equals("true")) {
                if (attr.multiplicity != null && !attr.multiplicity.equals("")) {
                    if (attr.type.equals("bool")) {
                        cpp_file += attr.type + " " + className + "::" + attr.name + "[" + attr.multiplicity + "]={false};\n";
                    } else {// int/double类型的数据初始化
                        cpp_file += attr.type + " " + className + "::" + attr.name + "[" + attr.multiplicity + "]={0};\n";
                    }
                } else {
                    cpp_file += attr.type + " " + className + "::" + attr.name + "=" + i + ";\n";
                    i++;
                }
            }
        }
//        System.out.println(cpp_file);
        cpp_file += add_construct();
        cpp_file += add_destructor();
    }
    public String add_construct() {
        Map<String,List<ClassAttribute>> initStructs = new HashMap<>();
        HashMap<String,String> map = globalElements.getTypeMap();
        constructor+= className + "::";
        List<ClassAttribute> parameters = new ArrayList<>();
        for (ClassFunction func:self_defined_funcs){
            if (func.name.equals(className)){
                constructor += func.name+"(";
                for (ClassAttribute attr:func.paraList){
                    constructor += attr.type+" "+attr.name+",";
                    List<Element> structs = globalElements.getStructs();
                    for (Element struct:structs){
                        String listType = attr.type.substring(attr.type.lastIndexOf("<")+1,attr.type.length()-1);
                        if (attr.type.contains("list") && listType.equals(struct.attributeValue("name"))){
                            ClassAttribute para = new ClassAttribute();
                            para.name = attr.name;
                            para.type = listType;
                            parameters.add(para);
                            List<ClassAttribute> attrs = new ArrayList<>();
                            List<Element> properties = findChildElementByAttribute(struct, "type", "uml:Property");
                            for (Element prop:properties){
                                ClassAttribute classAttribute = new ClassAttribute();
                                classAttribute.name = prop.attributeValue("name");
                                classAttribute.type = map.get(prop.attributeValue(new QName("type")));
                                attrs.add(classAttribute);
                            }
                            initStructs.put(listType,attrs);
                        }
                    }
                }
                constructor = constructor.substring(0,constructor.lastIndexOf(","));
                constructor += "){\n\t";
                if (func.body==null){
                    // 代码转移到// 初始化并添加子组件
                }
                break;
            }
        }
        for (ClassAttribute para:parameters){
            constructor += "list<"+para.type+">::iterator "+para.name+"_iterator = "+para.name+".begin();\n\t";
        }

        // 初始化并添加子组件
        List<Element> classElementList = globalElements.getClassElementList();
        for (ClassAttribute attr:attrs) {
            for (Element tmp : classElementList) {// 原子模型组件
                if (tmp.attributeValue("name").equals(attr.type)) {
                    String attr_name=attr.name;
                    if (attr.multiplicity!=null){
                        attr_name += "[i]";
                        constructor += "for (int i=0; i<"+attr.multiplicity+";i++){\n\t\t";
                        List<Element> funcList = tmp.elements("ownedOperation");
                        assert funcList != null;
                        for (Element fun : funcList) {
                            if (fun.attributeValue("name") != null && fun.attributeValue("name").equals(attr.type)) {
                                List<Element> paras = fun.elements("ownedParameter");
                                Set<String> keys = initStructs.keySet();
                                for (String key:keys){
                                    if (paras.size()==initStructs.get(key).size()){
                                        for (ClassAttribute classAttr:parameters){
                                            if (key.equals(classAttr.type)){
                                                constructor += "if ("+classAttr.name+"_iterator != "+classAttr.name+".end()){\n\t\t\t";
                                                List<ClassAttribute> values = initStructs.get(key);
                                                for (ClassAttribute classAttribute:values){
                                                    constructor += classAttribute.type + " "+ classAttribute.name;
                                                    constructor += " = "+classAttr.name+"_iterator->"+classAttribute.name+";\n\t\t\t";
                                                }
                                                constructor += classAttr.name+"_iterator++;\n\t\t\t";
                                            }
                                        }

                                    }
                                }
                            }
                        }
                    }
                    constructor += attr_name + " = new "+ attr.type+"(";
                    List<Element> funcList = tmp.elements("ownedOperation");
                    assert funcList != null;
                    for (Element fun : funcList) {
                        if (fun.attributeValue("name") != null && fun.attributeValue("name").equals(attr.type)) {
                            List<Element> ownedParameters = fun.elements("ownedParameter");
                            for (Element para : ownedParameters) {
                                constructor += para.attributeValue("name")+ ",";
                            }
                            constructor = constructor.substring(0,constructor.lastIndexOf(","));
                            constructor += ");\n\t";
                            if (!parameters.isEmpty()) {
                                constructor += "\t";
                            }
                            if (isContinuousModel(globalElements.getRoot(), tmp)) {
                                constructor += "ode_solver<IO_Type>* ode_solver;\n\t";
                                if (attr.multiplicity!=null){
                                    constructor += "\t";
                                }
                                constructor += "ode_solver = new corrected_euler<IO_Type>(";
                                if (attr.multiplicity!=null){
                                    constructor += "\t";
                                }
                                constructor += attr_name +", 1E-1, 0.02);\n\t";
                                if (attr.multiplicity!=null){
                                    constructor += "\t";
                                }
                                constructor += "event_locator<IO_Type>* event_find = new bisection_event_locator<IO_Type>(";
                                constructor += attr_name +", 500);\n\t";
                                if (attr.multiplicity!=null){
                                    constructor += "\t";
                                }
                                constructor += "m_hybrid_"+ attr_name + "= new Hybrid<IO_Type>(";
                                constructor += attr_name + ", ode_solver, event_find);\n\t";
                                if (attr.multiplicity!=null){
                                    constructor += "\t";
                                }
                                constructor += "add(m_hybrid_"+ attr_name + ");\n\t";
                            }
                            else{
                                if (attr.multiplicity!=null){
                                    constructor += "\t";
                                }
                                constructor += "add("+ attr_name + ");\n\t";
                                if (!parameters.isEmpty()) {
                                    constructor += "\t}\n\t";
                                }
                            }
                            if (attr.multiplicity!=null){
                                constructor += "}\n\t";
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
        // 构建连接couple
        constructor+="\n\t";
        for (Connector conn:connectors){
            String hybridSrc = conn.src;
            String src = conn.src;
            String tar = conn.tar;
            String hybridTar = conn.tar;
            String srcType = conn.srcType;
            String tarType = conn.tarType;
            for (Element clazz:globalElements.getClassElementList()){
                if (clazz.attributeValue("name").equals(srcType) && isContinuousModel(globalElements.getRoot(),clazz)){
                    hybridSrc = "m_hybrid_"+hybridSrc;
                }
                if (clazz.attributeValue("name").equals(tarType) && isContinuousModel(globalElements.getRoot(),clazz)){
                    hybridTar = "m_hybrid_"+hybridTar;
                }
            }
            boolean isSrcs = false;
            boolean isTars = false;
            for (ClassAttribute attr:attrs) {
                if (attr.name.equals(conn.src) && attr.multiplicity!=null){
                    constructor+="for (int i = 0; i < "+attr.multiplicity+"; i++) {\n\t";
                    hybridSrc += "[i]";
                    src += "[i]";
                    isSrcs = true;
                }
                else if (attr.name.equals(conn.tar) && attr.multiplicity!=null){
                    constructor+="for (int j = 0; j < "+attr.multiplicity+"; j++) {\n\t";
                    tar += "[j]";
                    hybridTar += "[j]";
                    isTars = true;
                }
                if (isSrcs){
                    constructor+="\t";
                }
                if (isTars){
                    constructor+="\t";
                }
            }
            constructor+="couple("+hybridSrc+","+src+"->"+conn.srcPort+","+hybridTar+","+tar+"->"+conn.tarPort+");\n\t";
            if (isTars){
                constructor+="\t}\n\t";
            }
            if (isSrcs){
                constructor+="}\n\t";
            }
        }
        constructor+="\n}\n";
//        System.out.println(constructor);
        return constructor;
    }
    public String add_destructor() {
        destructor+=className + "::~" + className + "()\n{\n";
        destructor+="}\n\n";
        return destructor;
    }
    public void findConnector(){
        List<Element> conns = model.elements("ownedConnector");
        for(Element conn : conns){
            Connector connector = new Connector();
            HashMap<String,String> map = globalElements.getTypeMap();
            List<Element> end2 = conn.elements("end");
            for (int i=0; i<end2.size(); i++){
                String portId = end2.get(i).attributeValue("role");// 端口0
                List<Element> ports = findAllChildElementByAttributeText(globalElements.getModel(), "id",portId );
                boolean isOut = false;
                if(ports!=null && ports.size() > 0){
                    List<Element> flowPorts= findChildElementByAttribute(globalElements.getRoot(), "base_Port",portId );
                    if (flowPorts.size()>0 && flowPorts.get(0).attributeValue("direction").equals("in")){
                        connector.tarPort = ports.get(0).attributeValue("name");
                        isOut = false;
                    }else if(flowPorts.size()>0 && flowPorts.get(0).attributeValue("direction").equals("out")){
                        connector.srcPort = ports.get(0).attributeValue("name");
                        isOut = true;
                    }
                    connector.portType = map.get(ports.get(0).attributeValue(new QName("type")));
                }
                String partWithPortId = end2.get(i).attributeValue("partWithPort");// 组件0
                if (partWithPortId!=null){
                    List<Element> parts = findChildElementByAttribute(this.model, "id", partWithPortId);
                    if(parts.size() > 0){
                        if (isOut){
                            connector.src = parts.get(0).attributeValue("name");
                            connector.srcType = map.get(parts.get(0).attributeValue(new QName("type")));
                        }else{
                            connector.tar = parts.get(0).attributeValue("name");
                            connector.tarType = map.get(parts.get(0).attributeValue(new QName("type")));
                        }
                    }
                }else{
                    if (isOut){
                        connector.tar = "this";
                        connector.tarPort = connector.srcPort;
                        connector.tarType = className;
                    }else {
                        connector.srcPort = connector.tarPort;
                        connector.src = "this";
                        connector.srcType = className;
                    }
                }
            }
            connectors.add(connector);
         }
        globalElements.addConnectors(connectors);
    }
    public void findFunctions() {
        List<Element> funcList = model.elements("ownedOperation");
        assert funcList != null;
        for (Element func : funcList) {
            ClassFunction classFunction = new ClassFunction();
            classFunction.paraList = new ArrayList<>();
            if (func.attributeValue("name") != null) {
                classFunction.name = func.attributeValue("name");
            }
            List<Element> parameters = func.elements("ownedParameter");
            HashMap<String,String> map = globalElements.getTypeMap();
            for (Element para : parameters) {
                if (para.attributeValue("direction")!=null && para.attributeValue("direction").equals("return")){// 返回值
                    if (map.get(para.attributeValue(new QName("type")))!=null){
                        classFunction.returnType = map.get(para.attributeValue(new QName("type")));
                    }
                    else {
                        List<Element> referenceExtension = findAllChildElementByAttributeText(para, "referentType", "DataType");
                        assert referenceExtension != null;
                        if (referenceExtension.size()>0){
                            String type = referenceExtension.get(0).attributeValue("referentPath");
                            type = type.substring(type.lastIndexOf("::")+2,type.length());
                            classFunction.returnType = type;
                        }
                    }
                }else {// 参数列表
                    ClassAttribute attribute = new ClassAttribute();
                    attribute.name = para.attributeValue("name");
                    if (map.get(para.attributeValue(new QName("type")))!=null){
                        attribute.type = map.get(para.attributeValue(new QName("type")));
                    }else{
                        System.out.println("参数类型缺失！"+classFunction.name);
                    }
                    // 除了基本属性还有isStatic和readOnly
                    classFunction.paraList.add(attribute);
                }
            }
            if (func.element("method") != null && func.element("method").attributeValue("idref") != null) {
                List<Element> method = findChildElementByAttribute(model, "id", func.element("method").attributeValue("idref"));
                if (method.size()>0){
                    classFunction.body = method.get(0).element("body").getText();
                }
            }
            self_defined_funcs.add(classFunction);
        }
    }
    public void findAttributes() {
        List<Element> ports = findChildElementByAttribute(model, "type", "uml:Port");
        HashMap<String,String> map = globalElements.getTypeMap();
        assert ports != null;
        for (Element tmp : ports) {
            if (map.get(tmp.attributeValue(new QName("type")))!=null) {
                ClassAttribute attr = new ClassAttribute();
                attr.name = tmp.attributeValue("name");
                attr.type = "int";
                attr.isStatic = "true";
                attrs.add(attr);
            }
        }
        List<Element> properties = findChildElementByAttribute(model, "type", "uml:Property");
        assert properties != null;
        for (Element tmp : properties) {
            if (map.get(tmp.attributeValue(new QName("type")))!=null){
                ClassAttribute attr = new ClassAttribute();
                attr.name = tmp.attributeValue("name");
                attr.type = map.get(tmp.attributeValue(new QName("type")));
                if (tmp.attributeValue("isStatic")!=null){
                    attr.isStatic = tmp.attributeValue("isStatic");
                }
                if (tmp.element("lowerValue")!=null){
                    attr.multiplicity = tmp.element("lowerValue").attributeValue("value");
                }
                attrs.add(attr);
            }

        }

    }
    //判断是否是连续模型，只要有一个ConstraintBlock property，那他就是连续模型
    private boolean isContinuousModel(Element root, Element element){
        List<Element> properties = findChildElementByAttribute(element, "type", "uml:Property");
        for(Element property : properties){
            if(property.attributeValue(new QName("type")) != null){
                List<Element> bC = findChildElementByAttribute(root, "base_Class", property.attributeValue(new QName("type")));
                if(bC.size() != 0 && bC.get(0).getName() == "ConstraintBlock"){
                    return true;
                }
            }
        }
        return false;
    }
    public void writeToFile() throws IOException {
        String fileDir = globalElements.getDirPath();
        File head = new File(fileDir+ "/" + className + ".h");
        BufferedWriter fw_head = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(head),"unicode"));
        fw_head.write(head_file.toString());
        fw_head.flush();
        fw_head.close();
        File cpp = new File(fileDir+ "/" + className + ".cpp");
        BufferedWriter fw_cpp = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(cpp),"unicode"));
        fw_cpp.write(cpp_file.toString());
        fw_cpp.flush();
        fw_cpp.close();
    }
}

package xmlTools;

import org.dom4j.Element;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class GeneralParseXMLMethod {
//    private static Element model;//模型树根节点元素Model
    private static Element extension;//视图部分根节点元素
    private Element dataModel;
    private Element activityModel;
    private List<Element> activityModelList;
    private static List<Element> classElementList;

    public GeneralParseXMLMethod(Element root) {
//        model = findChildElementByName(root, "Model").get(0);
//        extension = findChildElementByName(root, "Extension").get(1);//xml文件中有2个Extension节点，第二个是视图部分
//        this.classElementList =  findAllChildElementByAttributeText(model, "type", "uml:Class");
    }

    /*
   只能查找下一级的子节点——根据节点名称
    */
    public static List<Element> findChildElementByName(Element element, String elementName) {
        List<Element> elementList = new ArrayList<Element>();

        for (Iterator<Element> elementIterator = element.elementIterator(elementName); elementIterator.hasNext();) {
            Element elementChild = elementIterator.next();
//            System.out.println("元素：" + elementChild.getName());
//            System.out.println(elementChild.attributeValue("name"));
            elementList.add(elementChild);
        }
        return elementList;
    }

    /*
    只能查找下一级的子节点——根据属性
     */
    public static List<Element> findChildElementByAttribute(Element element, String attributeName, String attributeText) {
        List<Element> elementList = new ArrayList<Element>();

        if (element == null) {//20201105 针对BUG-java.lang.NullPointerException
            return elementList;
        }

        for (Iterator<Element> it = element.elementIterator(); it.hasNext();) {
            Element elementChild = it.next();

//            if (elementChild.attributeValue(attributeName) != null && elementChild.attributeValue("name")!=null)
//                System.out.println(attributeName + elementChild.attributeValue(attributeName) + "\tname:" + elementChild.attributeValue("name") );
            if (elementChild.attributeValue(attributeName) != null && elementChild.attributeValue(attributeName).equals(attributeText)
            || elementChild.attributeValue(attributeName) != null && attributeText.equals("not null is ok")) {
                elementList.add(elementChild);
//                System.out.println(attributeName + elementChild.attributeValue(attributeName) + "\tname:" + elementChild.attributeValue("name") );
            }
        }
        return elementList;
    }

    /*
    新版本：查找所有子节点——根据属性
     */
    private static List<Element> allElementList = new CopyOnWriteArrayList<Element>();//尝试使用CopyOnWriteArrayList解决ERROR-java.util.ConcurrentModificationException
    private static void findAllChildElementByAttribute(Element element, String attributeName, String attributeText) {

        for (Iterator<Element> it = element.elementIterator(); it.hasNext();) {
            Element elementChild = it.next();

            if (elementChild.attributeValue(attributeName) != null) {
//                System.out.println(attributeName + elementChild.attributeValue(attributeName) + "\tname:" + elementChild.attributeValue("name") );
                if (attributeText.equals("not null is ok")) {//适配视图部分
                    allElementList.add(elementChild);
                }
                else if (elementChild.attributeValue(attributeName).equals(attributeText)) {
                    allElementList.add(elementChild);
                }
            }
            findAllChildElementByAttribute( elementChild,  attributeName,  attributeText);

        }
    }
    public static List<Element> findAllChildElementByAttributeText(Element element, String attributeName, String attributeText) {
//        System.out.println(allElementList.size() + attributeName);
        allElementList.clear();
//        System.out.println(allElementList.size() + attributeName);
        if (element == null) {
            return null;
        }
        findAllChildElementByAttribute( element,  attributeName,  attributeText);
//        System.out.println(allElementList.size() + attributeText);
//        for (Element tmp : allElementList) {
//            System.out.println(tmp.attributeValue("id")+tmp.attributeValue("name"));
//        }
        return allElementList;
    }

    public static List<Element> findElementInListByAttribute(List<Element> element, String attributeName, String attributeText) {
        List<Element> elementList = new ArrayList<Element>();

        for (Element elementChild : element) {
            if (elementChild.attributeValue(attributeName) != null
                    && elementChild.attributeValue(attributeName).equals(attributeText)) {
                elementList.add(elementChild);
//                System.out.println(elementChild.attributeValue(attributeName));
            }
        }
        return elementList;
    }

    //将functionName用::连接name
    public static List<String> iteratorGetTextOfNameAttribute(List<Element> elementList, String functionName) {
        List<String> Parameter = new ArrayList<String>();
        Iterator<Element> elementIterator = elementList.iterator();
        while (elementIterator.hasNext()) {
            Element elementArgument = elementIterator.next();
            Parameter.add(functionName + "::" + elementArgument.attributeValue("name"));
        }
//        System.out.println("链接后的返回值：" + Parameter);
        return Parameter;
    }

    /*
    旧版本：查找唯一子节点——根据唯一属性——id
    可由新版本查找所有子节点重构
     */
    static boolean flagId = false;
    static Element falgIdElement = null;
    public static Element getElementByIdMain(Element begin, String id, String idName){
        flagId = false;
        falgIdElement = null;
        return getElementById(begin, id, idName);
    }
    public static Element getElementById(Element elementRoot, String id, String idName){
        List<Element> elements = elementRoot.elements();
        for (Element element : elements) {
            if (element.attributeValue("type") != null && element.attributeValue(idName) != null
                    && element.attributeValue(idName).equals(id)) {
                flagId = true;
                falgIdElement = element;
                return falgIdElement;
            }
            getElementById(element, id, idName);//递归
            if (flagId == true) {
                return falgIdElement;
            }
        }
        return falgIdElement;
    }



//    public static Element getModel() {
//        return model;
//    }

//    public void setModel(Element model) {
//        this.model = model;
//    }

    public Element getDataModel() {
        return dataModel;
    }

    public void setDataModel(Element dataModel) {
        this.dataModel = dataModel;
    }

    public Element getActivityModel() {
        return activityModel;
    }

    public void setActivityModel(Element activityModel) {
        this.activityModel = activityModel;
    }

    public List<Element> getActivityModelList() {
        return activityModelList;
    }

    public void setActivityModelList(List<Element> activityModelList) {
        this.activityModelList = activityModelList;
    }

    public static Element getExtension() {
        return extension;
    }

    public static void setExtension(Element extension) {
        GeneralParseXMLMethod.extension = extension;
    }


    public static List<Element> getClassElementList() {
        return classElementList;
    }

    public void setClassElementList(List<Element> classElementList) {
        this.classElementList = classElementList;
    }

//    public static Element getModel() {
//        return model;
//    }
//
//    public static void setModel(Element model) {
//        GeneralParseXMLMethod.model = model;
//    }
}

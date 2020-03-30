package com.example.labstatus.ui.SL1;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
/** @brief Model for SL1 grid data

Detailed description follows here.
 @author Area51
 @date Nov 2019
 */
public class SL1ViewModel extends ViewModel {

    private MutableLiveData<String> mText;
    public String text;

    public String color;

    public SL1ViewModel() {
        mText = new MutableLiveData<>();
        mText.setValue("This is SL1 fragment");

    }
    /**
     * Constructor for Model
     * @param c color
     * @param t text
     */
    public SL1ViewModel(String t,String c) {

        text=t;
        color=c;
    }

    public LiveData<String> getText() {
        return mText;
    }
}